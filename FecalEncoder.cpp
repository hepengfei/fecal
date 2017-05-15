/*
    Copyright (c) 2017 Christopher A. Taylor.  All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name of Fecal nor the names of its contributors may be
      used to endorse or promote products derived from this software without
      specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/

#include "FecalEncoder.h"

namespace fecal {


//------------------------------------------------------------------------------
// EncoderAppDataWindow

void EncoderAppDataWindow::AllocateOriginals()
{
    OriginalData.resize(InputCount);
}

void EncoderAppDataWindow::SetEncoderInput(void* const * const input_data)
{
    FECAL_DEBUG_ASSERT(InputCount > 0); // SetParameters() must be called first

    for (unsigned ii = 0, count = InputCount; ii < count; ++ii)
        OriginalData[ii] = reinterpret_cast<const uint8_t*>(input_data[ii]);
}


//------------------------------------------------------------------------------
// Encoder

FecalResult Encoder::Initialize(unsigned input_count, void* const * const input_data, uint64_t total_bytes)
{
    // Validate input and set parameters
    if (!Window.SetParameters(input_count, total_bytes))
    {
        FECAL_DEBUG_BREAK; // Invalid input
        return Fecal_InvalidInput;
    }
    Window.AllocateOriginals();
    Window.SetEncoderInput(input_data);

    const unsigned symbolBytes = Window.SymbolBytes;

    // Allocate lane sums
    for (unsigned laneIndex = 0; laneIndex < kColumnLaneCount; ++laneIndex)
    {
        for (unsigned sumIndex = 0; sumIndex < kColumnSumCount; ++sumIndex)
        {
            if (!LaneSums[laneIndex][sumIndex].Allocate(symbolBytes))
                return Fecal_OutOfMemory;

            // Clear memory in each lane sum
            memset(LaneSums[laneIndex][sumIndex].Data, 0, symbolBytes);
        }
    }

    // Allocate workspace
    if (!Sum.Allocate(symbolBytes) || !Product.Allocate(symbolBytes))
        return Fecal_OutOfMemory;

    // TBD: Unroll first set of 8 lanes to avoid the extra memset above?
    // TBD: Use GetLaneSum() approach do to minimal work for small output?

    // For each input column:
    for (unsigned column = 0; column < input_count; ++column)
    {
        const uint8_t* data = reinterpret_cast<const uint8_t*>(input_data[column]);
        const unsigned laneIndex = column % kColumnLaneCount;

        const uint8_t CX = GetColumnValue(column);
        const uint8_t CX2 = gf256_sqr(CX);

        // Sum[0] += Data
        gf256_add_mem(LaneSums[laneIndex][0].Data, data, symbolBytes);

        // Sum[1] += CX * Data
        gf256_muladd_mem(LaneSums[laneIndex][1].Data, CX, data, symbolBytes);

        // Sum[2] += CX^2 * Data
        gf256_muladd_mem(LaneSums[laneIndex][2].Data, CX2, data, symbolBytes);

        static_assert(kColumnSumCount == 3, "Update this");
    }

    return Fecal_Success;
}

FecalResult Encoder::Encode(unsigned row, FecalSymbol& symbol)
{
    // If encoder is not initialized:
    if (!Product.Data)
        return Fecal_InvalidInput;

    // Load parameters
    const unsigned count = Window.InputCount;
    const unsigned symbolBytes = Window.SymbolBytes;
    uint8_t* outputSum = Sum.Data;
    uint8_t* outputProduct = Product.Data;

    // Initialize LDPC
    PCGRandom prng;
    prng.Seed(row, count);

    // Accumulate original data into the two sums
    const unsigned pairCount = (Window.InputCount + kPairAddRate - 1) / kPairAddRate;
    // Unrolled first loop:
    {
        const unsigned element1 = prng.Next() % count;
        const uint8_t* original1 = Window.OriginalData[element1];

        const unsigned elementRX = prng.Next() % count;
        const uint8_t* originalRX = Window.OriginalData[elementRX];

        // Sum = Original[element1]
        if (Window.IsFinalColumn(element1))
        {
            memcpy(outputSum, original1, Window.FinalBytes);
            memset(outputSum + Window.FinalBytes, 0, symbolBytes - Window.FinalBytes);
        }
        else
            memcpy(outputSum, original1, symbolBytes);

        // Product = Original[elementRX]
        if (Window.IsFinalColumn(elementRX))
        {
            memcpy(outputProduct, originalRX, Window.FinalBytes);
            memset(outputProduct + Window.FinalBytes, 0, symbolBytes - Window.FinalBytes);
        }
        else
            memcpy(outputProduct, originalRX, symbolBytes);
    }

    XORSummer sum;
    sum.Initialize(outputSum, symbolBytes);
    XORSummer prod;
    prod.Initialize(outputProduct, symbolBytes);

    for (unsigned i = 1; i < pairCount; ++i)
    {
        const unsigned element1   = prng.Next() % count;
        const uint8_t* original1  = Window.OriginalData[element1];

        const unsigned elementRX  = prng.Next() % count;
        const uint8_t* originalRX = Window.OriginalData[elementRX];

        // Sum += Original[element1]
        if (Window.IsFinalColumn(element1))
            gf256_add_mem(outputSum, original1, Window.FinalBytes);
        else
            sum.Add(original1);

        // Product += Original[elementRX]
        if (Window.IsFinalColumn(elementRX))
            gf256_add_mem(outputProduct, originalRX, Window.FinalBytes);
        else
            prod.Add(originalRX);
    }

    // For each lane:
    for (unsigned laneIndex = 0; laneIndex < kColumnLaneCount; ++laneIndex)
    {
        // Compute the operations to run for this lane and row
        unsigned opcode = GetRowOpcode(laneIndex, row);

        // Sum += Random Lanes
        unsigned mask = 1;
        for (unsigned sumIndex = 0; sumIndex < kColumnSumCount; ++sumIndex, mask <<= 1)
            if (opcode & mask)
                sum.Add(LaneSums[laneIndex][sumIndex].Data);

        // Product += Random Lanes
        for (unsigned sumIndex = 0; sumIndex < kColumnSumCount; ++sumIndex, mask <<= 1)
            if (opcode & mask)
                prod.Add(LaneSums[laneIndex][sumIndex].Data);
    }

    sum.Finalize();
    prod.Finalize();

    // Sum += RX * Product
    gf256_muladd_mem(outputSum, GetRowValue(row), outputProduct, symbolBytes);

    // Output sum will be what user reads from
    symbol.Data = outputSum;
    symbol.Bytes = symbolBytes;
    symbol.Index = row;

    return Fecal_Success;
}


} // namespace fecal