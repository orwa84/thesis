#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <string.h>
#include <assert.h>

#include "randomizer.h"
#include "word_library.h"

// Enable this line to suppress solution details
#define SUPPRESS_DETAILS
#undef SUPPRESS_DETAILS

int main (int argc, const char * argv[]) {

	// initializing the randomizer ensures that the random bits
	// provided through random_bit will be different each time
	// the program is executed.
	if (-1 == initialize_randomizer()) return -1;
	
	// independent system parameters
	//   - SET 1: BASIC-THEORETICAL
	const unsigned short 
		algorithm_m = 2,
		algorithm_n = 10,
		algorithm_Z = 4;
	
	//   - SET 2: BASIC-PRACTICAL
	const unsigned short
		algorithm_alpha = 3,
		algorithm_beta = 3,
		algorithm_ns = 3,
		algorithm_np = 5,
		algorithm_np_fractional = 2,
		algorithm_table_unsigned = 1;
	
	const short SRT_table[][5] = {
		{4, 4, 4, 3},
		{4, 4, 4, 3},
		{4, 4, 4, 3},
		{4, 4, 4, 3},
		{4, 4, 3, 3},
		{4, 4, 3, 3},
		{4, 4, 3, 3},
		{4, 4, 3, 3},
		{4, 3, 3, 3},
		{4, 3, 3, 3},
		{4, 3, 3, 3},
		{4, 3, 3, 3},
		{3, 3, 3, 3},
		{3, 3, 3, 2},
		{3, 3, 3, 2},
		{3, 3, 3, 2},
		{3, 3, 3, 2},
		{3, 3, 2, 2},
		{3, 3, 2, 2},
		{3, 2, 2, 2},
		{3, 2, 2, 2},
		{3, 2, 2, 1},
		{2, 2, 2, 1},
		{2, 2, 2, 1},
		{2, 2, 2, 1},
		{2, 1, 1, 1},
		{2, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 0, 0, 0},
		{1, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0}
	};

	const unsigned short SRT_table_dimensions[] = {33,4};
	const unsigned short SRT_table_p0 = 8 << algorithm_np_fractional;
	
	const unsigned short 
		SRT_table_mappings[][3] = {{0,0,0}};
	const unsigned short 
		SRT_table_mappings_count = 0;
	
	
	// dependent system parameters
	// (following thesis notation)
	const unsigned short iterations =
		(unsigned short) algorithm_n + ceil((double) (algorithm_Z + 2) / algorithm_m);
	const unsigned short delta = 
		(unsigned short) floor((double) algorithm_Z / algorithm_m) + 1;
	//const unsigned short sigma = 
	//	(unsigned short) floor((double) (algorithm_Z + algorithm_ns) / algorithm_m) + 1;
	
	//const unsigned short ma = algorithm_Z - algorithm_m * floor((double) algorithm_Z / algorithm_m);
	const unsigned short mb = algorithm_m * ceil((double) algorithm_Z / algorithm_m) - algorithm_Z;
	
	// processor_size denotes the width of the calculations in
	// the processor which is also equal to the bit length of
	// different variables/operands.
	const unsigned short processor_size = algorithm_m * algorithm_n;
	word_pointer random_seed1 = NULL, random_seed2 = NULL;
	word_pointer A = NULL, B = NULL, S = NULL;
	
	// To result in an exact square root we require that the
	// choice of algorithm_m and algorithm_n result in an even
	// value for "processor_size".
	
	// the plan for producing random operands and an exact 
	// square root:
	//
	// A (multiplicand) = (1st random seed)^2
	// B (multiplier)   = (2nd random seed)^2
	//  A × B = (1st random seed × 2nd random seed)^2
	// √A × B = (1st random seed × 2nd random seed)
	//
	// note: since both "A" and "B" are required to have a
	// normalized size equal to "processor_size", and since
	// both are formed by the means of squaring a number
	// (doubling its bit length), we need "processor_size"
	// to be even (divisible by two).
	
	assert((1 & processor_size) == 0 && "PROCESSOR BIT SIZE SHOULD BE AN EVEN VALUE");
	if ((1 & processor_size) != 0) {
		perror("PROCESSOR BIT SIZE SHOULD BE AN EVEN VALUE");
		return 0;
	}
	
	// generate the half-size random seeds
	random_seed1 = create_word(processor_size >> 1);
	random_seed2 = create_word(processor_size >> 1);

	word_randomize(random_seed1);
	word_randomize(random_seed2);
	
	// compute the random operand values (the multiplier
	// B and multiplicand A)
	A = create_word(processor_size);
	B = create_word(processor_size);
	
	word_op_multiply(A, random_seed1, random_seed1);
	word_op_multiply(B, random_seed2, random_seed2);
	
	assert(!A->overflow && !A->underflow && "INCORRECT RANDOM GENERATION OF THE MULTIPLICAND A");
	assert(!B->overflow && !B->underflow && "INCORRECT RANDOM GENERATION OF THE MULTIPLIER B");	
	if (A->overflow || A->underflow || B->overflow || B->underflow) {
		return 0;
	}
	
	// compute the exact square root
	S = create_word(processor_size);

	word_op_multiply(S, random_seed1, random_seed2);
	assert(!S->overflow && !S->underflow && "INCORRECT RANDOM GENERATION OF THE SQUARE ROOT S");
	if (S->overflow || S->underflow) {
		return 0;
	}
	
	printf(
		"-----------------------------------------------------------------------------\n"		   
		"           ORWA-AMIN MULTIPLICATIVE SQUARE-ROOT ALGORITHM SIMULATOR          \n"
		"-----------------------------------------------------------------------------\n"
		" Configuration: <ADVANCED> \n"
		" - Signed-digit result with on-the-fly conversion of digits: YES\n"
		" - Supports rounding: YES\n"
		" - Actual digit selection: YES\n"
		" - Digit selection using an actual table: YES\n"
		" - Carry-Save residual: YES\n"
		" - Signed-digit multiplier B with automatic conversion: NO\n"
		"-----------------------------------------------------------------------------\n"
		   );
	
	printf("System parameters:\n"
		   " - m: %d bits\t\t→ RADIX = %d\n - n: %d iterations\n - Z: %d bits\n"
		   "\nSystem variables:\n"
		   " - size of operands:    %d bits\n"			// "m × n ="
		   " - iterations:			%d iteration(s)\n",	// "n + ⎡Z+2/m⎤ = "
		   algorithm_m, 1 << algorithm_m, algorithm_n, algorithm_Z, 
		   processor_size, iterations);
	
	char *buffer1, *buffer2, *buffer3;
	
	buffer1 = word_makestring(A, 1 << algorithm_m);
	buffer2 = word_makestring(B, 1 << algorithm_m);	
	buffer3 = word_makestring(S, 1 << algorithm_m);		
	printf(
#if !defined(SUPPRESS_DETAILS)
		   "-----------------------------------------------------------------------------\n"		   
		   " TYPE OF SIMULATION: One-problem demonstration S = √A × B\n"
		   "-----------------------------------------------------------------------------\n"
#else
		   "-----------------------------------------------------------------------------\n"		   
		   " TYPE OF SIMULATION: One-problem verification\n"
		   "-----------------------------------------------------------------------------\n"		   
#endif
		   "Problem data:\n"
		   " - A : %s (size = %d bits, radix = %d)\n"
		   " - B : %s (size = %d bits, radix = %d)\n"
		   " - S : %s (size = %d bits, radix = %d)\n",
		buffer1, processor_size, 1 << algorithm_m, 
		buffer2, processor_size, 1 << algorithm_m,
		buffer3, processor_size, 1 << algorithm_m);
		   
	free(buffer1);
	free(buffer2);
	free(buffer3);	
	
	// Prepare the digits of the precomuputed  square root to
	// feed them serially to the algorithm. Note that the square
	// root produced or demanded by the algorithm is the delayed
	// (S'), rather than the direct square root (S). The differe-
	// nce is that S' has Z leading zero bits as compared to S.
	word_pointer S_prime = create_word(
				algorithm_Z + processor_size + mb);
	word_op_load(S_prime, S, mb);
	
	unsigned int *S_prime_digits = word_makelist(S_prime, algorithm_m);
	
	/*
	assert(S_prime_digits[0] == (iterations) && 
		   "NOT ENOUGH RESULT DIGITS PRODUCED");
	if (S_prime_digits[0] != (iterations)) {
		perror("NOT ENOUGH RESULT DIGITS PRODUCED");
		return 0;
	}
	 */
	
	
	printf(" - S': %s (size = %d bits)\n\n",
		   buffer1 = word_makestring(S_prime, 1 << algorithm_m),
		   processor_size + algorithm_Z);

	free(buffer1);
	
	// Similarly, prepare the digits of the multiplier to be
	// consumed easily by the algorithm.
	unsigned int *B_digits = word_makelist(B, algorithm_m);
	
	// -------------------------------------
	// define all the hardware registers 
	// needed by the algorithm.
	// -------------------------------------
	const unsigned short register_S_size = 
		algorithm_m * (iterations) - algorithm_Z;
	const unsigned short register_A_size = 
		algorithm_m * (iterations + algorithm_n - 2);
	const unsigned short register_W_size = 
		algorithm_m * (iterations + algorithm_n + 2) + algorithm_Z + 1;
	
	word_pointer register_S = create_word(register_S_size);
	word_pointer register_A = create_word(register_A_size);
	word_pointer register_W = create_word(register_W_size);
	
	// This is the actual result register in a practical 
	// implementation of the algorithm, it is formed through
	// on-the-fly conversion of result digits as returned
	// by the SRT table.
	// (On-the-fly appending of signed digits require that
	// we maintain two registers, one to hold {S'} and the
	// other to hold {S'} - 1 -this is the one named "S_m1")
	word_pointer register_S_practical = create_word(register_S_size);	
	word_pointer register_S_m1 = create_word(register_S_size);	
	
	// Similar to the above, we maintain two registers to
	// hold 2{S'} and 2{S'} - 1, to ease the formation of the
	// value [2{S'}|s'] within the on-the-fly conversion unit,
	// which is needed for the formation of the linear-
	// quadratic term.
	word_pointer register_2S    = create_word(register_S_size + 1);	
	word_pointer register_2S_m1 = create_word(register_S_size + 1);
	
	// this is the actual residual register in a practical
	// implementation of the algorithm, which follows the
	// pace of the practical result register.
	word_pointer register_W_practical = create_word(register_W_size);
	
	// mechanisms needed for the extraction of the shifted 
	// partial residual P as required by digit-selection
	// (to be used to index the SRT table)
	word_pointer P_mask = create_word(register_W_size);
	// contains both the fractional and the integral parts
	// of the P sample (Ptruncated).
	word_pointer P = create_word(algorithm_np + 1 /* sign bit */ + 1 /* loose bit shift */);
	
	// the truncated fractional result "Sdot" which is needed
	// for indexing the SRT table.
	// the one is to account for the loose bit
	word_pointer Sdot = create_word(algorithm_ns + 1 /* loose-bit shift */);
	// the integral bit is not needed to index the table when
	// using a First-Digit Selector
	
	// the least-significant bit position of the residual sample,
	// known in the SRT division world as the partial shifted 
	// residual P.
	// the minus one is to account for the case when 2P has to
	// be passed to the table.
	unsigned short P_cursor = 
		(algorithm_m * algorithm_n + algorithm_Z) - algorithm_np_fractional - 1;
	// the mask needed to pass that sample out of the residual W
	word_op_load_constant(P_mask, ~0, P_cursor, 
						  algorithm_np + 1 /* sign bit */ + 1 /* loose bit shift */);
	
	// make the W register of the signed type, to ensure that
	// a potential case of overflow is correctly interpreted.
	register_W->is_signed = 1;
	// No carry-save format for the residual yet. So it is
	// also signed, and the sample P also.
	register_W_practical->is_signed = 1;
	P->is_signed = 1;

	word_pointer digit_multiplier_B = create_word(algorithm_m);
	word_pointer digit_multiplier_S = create_word(algorithm_m);	
	
	// The result digit returned by an actual SRT-table look
	// up. Note that unlike the precomputed digit above, this
	// digit is signed and hence needs an extra sign bit.
	word_pointer digit_multiplier_S_practical = create_word(algorithm_m + 1);		
	digit_multiplier_S_practical->is_signed = 1;

	// These are the unsigned digit values to be appended to
	// the current root Si-1 as part of the on-the-fly 
	// conversion circuitry logic.
	// Note that two digit values are needed to update Si-1,
	// one to update the direct amount and another one to 
	// update the amount minus 1.
	word_pointer onthefly_appended_digit = create_word(algorithm_m);
	word_pointer onthefly_appended_digit_m1 = create_word(algorithm_m);
	
	// These on the other hand are the unsigned m+1-bit values
	// to be appended to the current root Si-1 to result in
	// 2Si, an amount that will be used in the following 
	// iteration to construct the linear-quadratic term.
	// (this term is formed by concatenating 2Si-1 with si)
	// (note that t2 is read "times two")
	word_pointer onthefly_appended_digit_t2 = create_word(algorithm_m + 1);
	word_pointer onthefly_appended_digit_t2m1 = create_word(algorithm_m + 1);
	
	// These two selectors on the other hand specify whether
	// the direct value (=0) or the "minus one" variant (=1)
	// should be used for the formation of either the direct
	// value (select), or the "minus one" value (select_m1).
	unsigned char onthefly_select = 0, onthefly_select_m1 = 0;
	
	// -------------------------------------
	// initialization step of the algorithm
	// -------------------------------------
	
	// theoretical {S'} (register_S) is correctly initialized
	// to zero at this point.
	
	// Z should be greater than or equal to m, for the reason
	// mentioned in the following comment section.
	assert(algorithm_Z >= algorithm_m && "Z should be greater than or equal to m.");
	if (algorithm_Z < algorithm_m) {
		perror("Z should be greater than or equal to m.");
		return 0;
	}
	
	// Both {S'} (register_S_practical) and 2{S'} (register_2S)
	// are also correctly initialized to zero. Note that since
	// "Z" is not permitted to have a value of ZERO, we eliminate
	// the scenario in which {S'} has to be initialized to "1"
	// and 2{S'} has to be initialized to "10" binary (2).
	
	// As for keeping a correct relationship with the minus-one
	// (*_m1) copy of both registers, this is not necessary since
	// at iteration number floor(Z / m), a hardwired digit 
	// selection of "1" will be made, leading into the value of
	// the "*_m1" registers being discarded.
	
	// {A} = A
	word_op_load(register_A, A, 0);
	
	// digit_multiplier_B = b1
	word_op_load_constant(digit_multiplier_B, B_digits[1], 0, algorithm_m);
	
	// {W} = b1×A
	word_op_multiply(register_W, digit_multiplier_B, A);
	
	// {W}practical = b1×A
	word_op_multiply(register_W_practical, digit_multiplier_B, A);

#if !defined(SUPPRESS_DETAILS)
	
	// display algorithm's status
	printf("iteration 0 (initialization):\n"
		   "{S} = %s\n"
		   "{A} = %s\n      (overflow = %s, underflow = %s)\n"
		   "{W} = %s\n      (overflow = %s, underflow = %s)\n\n",
		   buffer1 = word_makestring(register_S_practical, 1 << algorithm_m),
		   buffer2 = word_makestring(register_A, 1 << algorithm_m),
		   register_A->overflow ? "YES" : "NO", register_A->underflow ? "YES" : "NO",
		   buffer3 = word_makestring(register_W_practical, 1 << algorithm_m),
		   register_W_practical->overflow ? "YES" : "NO", register_W_practical->underflow ? "YES" : "NO");
	
	free(buffer1);
	free(buffer2);

	char *delimiter = malloc(strlen(buffer3) + 7);
	strncpy(delimiter, "      ", 6);
	
	buffer1 = delimiter + 6;
	for (unsigned int i = 0; i < strlen(buffer3); ++i)
		*(buffer1++) = '-';
	
	free(buffer3);
	*buffer1 = '\0';
#endif
	
	// -------------------------------------
	// the algorithm's loop
	// -------------------------------------
	for (unsigned int iteration = 1; 
		 iteration <= iterations; ++iteration) {
		
		// extract the partial shifted residual P, contains both the
		// integral and fractional parts.
		word_op_extract(register_W_practical, P, P_cursor);
		
		// extract the truncated fractional result Sdot
		word_op_extract(register_S_practical, Sdot, 
						((short) iteration - 1) * algorithm_m - algorithm_Z - algorithm_ns - 1 /* loose-bit */);

		// load the next multiplier digit bi+1 into "digit_multiplier_B"
		if (iteration < B_digits[0])
			word_op_load_constant(digit_multiplier_B, B_digits[iteration + 1], 0, algorithm_m);
		else word_op_load_constant(digit_multiplier_B, 0, 0, algorithm_m);
		
		// load the current delayed root digit s'i into "digit_multiplier_S"
		word_op_load_constant(digit_multiplier_S, 
							  (iteration <= S_prime_digits[0] ? S_prime_digits[iteration] : 0), 0, algorithm_m);
		
#if !defined(SUPPRESS_DETAILS)		
		printf("iteration %u (", iteration);
		
		if (iteration < B_digits[0]) {
			// maximum supported radix has 3 decimal digits per high-radix 
			// digit, plus a null character.
			buffer1 = malloc(4);
			sprintf(buffer1, "%.0f", word_approximatevalue(digit_multiplier_B));
			
			printf("b = %s", buffer1);
			
			free(buffer1);
		}
		
		if (iteration >= delta && iteration < B_digits[0])
			printf(", ");
		
		if (iteration >= delta) {
			printf("s'<precomputed> = %d", 
				   (iteration <= S_prime_digits[0] ? S_prime_digits[iteration] : 0));
		}
			   
		puts("):");

		register_W_practical->is_signed = 0;
		buffer1 = word_makestring(register_W_practical, 1 << algorithm_m);
		register_W_practical->is_signed = 1;	
		
		buffer2 = word_makestring(P_mask, 1 << algorithm_m);

		printf("      %s\n", buffer1);
		printf("(^) : %s\n", word_cleanstring(buffer2));
		printf("%s\n", delimiter);
		
		free(buffer2);
		
		P->is_signed = 0;
		buffer2 = word_makestring(P, 1 << algorithm_m);
		P->is_signed = 1;
		buffer3 = word_makestring(P, 1 << algorithm_m);
		
		printf("P  = \"%s\" (%c%s)\n", buffer2, word_sign(P), buffer3);
		
		free(buffer2);
		free(buffer3);
		
		{
			word_pointer temp = create_word(algorithm_ns + 1 + (algorithm_ns + 1) % algorithm_m);
			word_op_load(temp, Sdot, 0);
			
			word_op_leftshift(temp, (algorithm_ns + 1) % algorithm_m);
			
			buffer2 = word_makestring(temp, 1 << algorithm_m);
			word_deallocate(temp);
		}
		
		printf("S. = \"0\".\"%s\"\n\n", buffer2);
		
		free(buffer1);
		free(buffer2);	
#endif
		
		// THE SRT TABLE LOOK-UP
		{
			// the resulting digit of the look-up
			short signed_digit = 0;
			
			// First-Digit Selector: digit has to be chosen from {1,2,3}
			if (iteration == delta) {
				word_pointer W_sample = create_word(3);
				word_op_extract(register_W_practical, W_sample, 
								(algorithm_m * (algorithm_n + 1) + 2 * algorithm_Z) - 3);

				// ABC = 011 or more
				if (1 == word_op_compare_constant(W_sample, 2))
					signed_digit = 3;
				// ABC = 001 or more
				else if (1 == word_op_compare_constant(W_sample, 0))
					signed_digit = 2;
				// ABC = 000 		
				else
					signed_digit = 1;

				
#if !defined(SUPPRESS_DETAILS)	
				printf("s' = FIRST-DIGIT-SELECTOR(ABC = %c%c%c) = \"%d\"\n",
					 (BITS(W_sample)[2] ? '1' : '0'),
					 (BITS(W_sample)[1] ? '1' : '0'),
					 (BITS(W_sample)[0] ? '1' : '0'),
					 (int) signed_digit);
#endif
				word_deallocate(W_sample);

			} else if (iteration > delta) {
				
				// the signed value of P
				int Pregion = 0;
				// tells whether the 1st complement of Pregion was 
				// substituted for an otherwise negative value in
				// a system based on a symmetric lookup table.
				unsigned char was_inverted = 0;
				
				// the indices needed for the look-up
				unsigned int Sregion_index = 0, Pregion_index = 0;
				
				// The loose-bit signal, used to determine whether S/P or
				// 2S/2P need to be passed to the table.
				unsigned char loose_bit_signal = BITS(Sdot)[algorithm_ns];
				
				// (loose-bit signal is 1, means Sdot = 0.1XXXX)
				//		in this case, we sample the 2nd to the ns'th fractional position
				// (loose-bit signal is 0, means Sdot = 0.01XXXX)
				//		in this case, we sample the 3rd to the (ns+1)'th fractional position				
				for (int i = algorithm_ns - 1 - (1 - loose_bit_signal); i >= loose_bit_signal; --i) {
					Sregion_index <<= 1;
					Sregion_index += BITS(Sdot)[i];
				}
				
				// the translation needed when initialroot is 0
//				if (0 == algorithm_initialroot) ++Sregion_index;
				
				// calculation of the P region (not yet the index)
				// note that P is (np + 1) bits long, where the most-sig-
				// nificant bit is the sign bit, which is followed by np
				// amplitude bits.
				// upgrade: now P is (np + 2) bits long, to include an
				// extra bit for the loose-bit shifting.
				for (int i = algorithm_np + loose_bit_signal; i >= loose_bit_signal; --i) {
					Pregion <<= 1;
					
					// The most-significant bit is the sign bit
					if (i == algorithm_np + loose_bit_signal)
						Pregion -= BITS(P)[i];
					else Pregion += BITS(P)[i]; 
				}
				
				// for symmetric-table implementations
				if (algorithm_table_unsigned && Pregion < 0) {
					//Pregion = ~Pregion;
					Pregion = -Pregion;
					was_inverted = 1;
				}
				
				// calculation of the P table index
				Pregion_index = SRT_table_p0 - Pregion;
				
				// apply the custom mappings
				if (SRT_table_mappings_count > 0) {
					for (unsigned int i  = 0; i < SRT_table_mappings_count; ++i) {
						if (iteration == SRT_table_mappings[i][2] &&
							Sregion_index == SRT_table_mappings[i][0] - 1) {
							Sregion_index = SRT_table_mappings[i][1] - 1;
						}
					}
				}
				
				assert(Pregion_index < SRT_table_dimensions[0] && 
					   Sregion_index < SRT_table_dimensions[1] && 
					   "SRT table indexing problem (indices out of range).");
				if (Pregion_index >= SRT_table_dimensions[0] || 
					Sregion_index >= SRT_table_dimensions[1]) {
					perror("SRT table is not being indexed correctly "
						   "(one of the indices or both are out of range).");
					return 0;
				}
				
				// The actual look up
				signed_digit = SRT_table[Pregion_index][Sregion_index];
				
				// for symmetric-table implementations
				if (algorithm_table_unsigned && was_inverted) {
					signed_digit = -signed_digit;
				}
				
				assert(signed_digit >= -algorithm_alpha && 
					   signed_digit <= +algorithm_beta &&
					   "Forbidden cells of the SRT table are being invoked "
					   "by the SRT table look-up mechanism!");
				if (signed_digit < -algorithm_alpha ||
					signed_digit > +algorithm_beta) {
					perror("Access to forbidden areas of the SRT table was detected.");
					return 0;
				}
				
#if !defined(SUPPRESS_DETAILS)	
				printf("loose-bit signal = %s\n\n", 
					   loose_bit_signal ? "1" : "0");				
				printf("s' = SRTLookUp[%u][%u] = %d\n\n", 
					   Pregion_index, Sregion_index, signed_digit);
#endif
			} else {
#if !defined(SUPPRESS_DETAILS)	
				printf("s' = 0\n\n");
#endif
			}
	
			
			// The (in-range) signed digit returned by the SRT table look up.
			word_op_load_constant(digit_multiplier_S_practical, signed_digit, 0, 
								  algorithm_m + 1);
			
			
			// ON-THE-FLY CONVERSION: PART 1
			
			// the unsigned digits obtained from the on-the-fly conversion
			// circuitry. To be appended to either S or S_m1 to form the
			// next result word, or to be appended to either 2S or 2S_m1
			// to form the part [S|0|s] needed for the formation of the
			// linear-quadratic term.
			
			// onthefly_appended_digit/onthefly_select
			if (signed_digit >= 0) {
				onthefly_select = 0;
				word_op_load_constant(onthefly_appended_digit, 
					signed_digit, 0, algorithm_m);
				word_op_load_constant(onthefly_appended_digit_t2, 
					signed_digit << 1, 0, algorithm_m + 1);
			} else {
				onthefly_select = 1;
				word_op_load_constant(onthefly_appended_digit, 
					(1 << algorithm_m) + signed_digit, 0, algorithm_m);
				word_op_load_constant(onthefly_appended_digit_t2, 
					((1 << algorithm_m) + signed_digit) << 1, 0, algorithm_m + 1);
			}

			// onthefly_appended_digit_m1/onthefly_select_m1
			if (signed_digit > 0) {
				onthefly_select_m1 = 0;
				word_op_load_constant(onthefly_appended_digit_m1, 
					signed_digit - 1, 0, algorithm_m);
				word_op_load_constant(onthefly_appended_digit_t2m1, 
					((signed_digit - 1) << 1) + 1, 0, algorithm_m + 1);
			} else {
				onthefly_select_m1 = 1;
				word_op_load_constant(onthefly_appended_digit_m1, 
					(1 << algorithm_m) + signed_digit - 1, 0, algorithm_m);
				word_op_load_constant(onthefly_appended_digit_t2m1, 
					(((1 << algorithm_m) + signed_digit - 1) << 1) + 1, 0, algorithm_m + 1);
			}
		}
		
		// ON-THE-FLY CONVERSION: PART 2
		
		// formation of the [2{S'}|s'] value on the fly, which will be 
		// referred to as the S0s value (knowing that it can be written
		// as [S'|0|s']).
		word_pointer S0s = create_word(algorithm_m * (algorithm_n + 1));
		word_op_load(S0s, (0 == onthefly_select ? 
						   register_2S : register_2S_m1), algorithm_m);
		word_op_load(S0s, onthefly_appended_digit, 0);
		
		// Definition of both the partial-product (+) and the linear-
		// quadratic (-) terms.
		word_pointer partial_product_term = create_word(register_W_size);
		word_pointer linearquadratic_term = create_word(register_W_size);
		word_pointer linearquadratic_term_practical = create_word(register_W_size);	
		
		// unlike the linear-quadratic term in the theoretical case, 
		// the practical version has to be signed as it contains the
		// result of multiplying the (now signed) result digit with
		// the S0s word.
		linearquadratic_term_practical->is_signed = 1;
		
		// construct the partial product term
		word_op_multiply(partial_product_term, digit_multiplier_B, register_A);
		word_op_leftshift(partial_product_term, algorithm_m);
		
		// construct the linear-quadratic term
		word_op_load(linearquadratic_term, register_S, algorithm_m + 1);
		word_op_load_constant(linearquadratic_term, (iteration <= S_prime_digits[0] ? 
													 S_prime_digits[iteration] : 0), 0, algorithm_m);
		word_op_multiply(linearquadratic_term, digit_multiplier_S, linearquadratic_term);
		word_op_leftshift(linearquadratic_term, algorithm_m * (algorithm_n + 1) + 2 * algorithm_Z);

		// construct the practical linear-quadratic term
		word_op_multiply(linearquadratic_term_practical, 
						 digit_multiplier_S_practical, S0s);
		word_op_leftshift(linearquadratic_term_practical, 
						  algorithm_m * (algorithm_n + 1) + 2 * algorithm_Z);
	
#if !defined(SUPPRESS_DETAILS)	
		
		buffer1 = word_makestring(S0s, 1 << algorithm_m);
		buffer2 = word_makestring(digit_multiplier_S_practical, 1 << algorithm_m);
		buffer3 = word_makestring(register_2S, 1 << algorithm_m);
		
		//printf("// %c%s × %s (%s|%s)\n",
		//	   word_sign(digit_multiplier_S_practical), buffer2, buffer1,
		//	   buffer3, buffer2);
		
		free(buffer1);
		free(buffer2);
		free(buffer3);
		
		// display both terms
		buffer1 = word_makestring(partial_product_term, 1 << algorithm_m);
		linearquadratic_term_practical->is_signed = 0;
		buffer2 = word_makestring(linearquadratic_term_practical, 1 << algorithm_m);
		linearquadratic_term_practical->is_signed = 1;	
		
		register_W_practical->is_signed = 0;
		buffer3 = word_makestring(register_W_practical, 1 << algorithm_m);
		register_W_practical->is_signed = 1;

		printf("      %s\n", buffer3);
		
		// display the terms with proper alignment and clean display, 
		// eliminating unnecessary zeros
		{
			size_t W_string_size = strlen(buffer3);
			char buffer4[64] = {0};
			
			sprintf(buffer4, "( ) : %%%ds\n", (int) W_string_size - 1);

			if (iteration < B_digits[0]) {
				buffer4[1] = '+';
				printf(buffer4, word_cleanstring(buffer1));
			}
			
			if (iteration >= delta) {
				buffer4[1] = '-';
				printf(buffer4, word_cleanstring(buffer2));
			}
		}
		
		free(buffer1);
		free(buffer2);
		free(buffer3);				
#endif
		
		// now use both terms to update the residual word
		
		// update the residual register {W}
		word_op_leftshift(register_W, algorithm_m * 2);
		
		// update the practical residual register {W}
		word_op_leftshift(register_W_practical, algorithm_m * 2);
				
		word_op_add(register_W, partial_product_term, +1, 0);
		word_op_add(register_W_practical, partial_product_term, +1, 0);
		
#if !defined(SUPPRESS_DETAILS)		
		register_W_practical->is_signed = 0;
		buffer1 = word_makestring(register_W_practical, 1 << algorithm_m);
		register_W_practical->is_signed = 1;				
#endif
		
		word_op_add(register_W, linearquadratic_term, -1, 0);
		word_op_add(register_W_practical, linearquadratic_term_practical, -1, 0);	
		
#if !defined(SUPPRESS_DETAILS)		
		register_W_practical->is_signed = 0;
		buffer2 = word_makestring(register_W_practical, 1 << algorithm_m);
		register_W_practical->is_signed = 1;
		
		{
			word_pointer B_effective = create_word_duplicate(B);
			word_pointer result_squared = create_word(processor_size << 1);			
			
			word_pointer register_W_correct = create_word(register_W_size);
			register_W_correct->is_signed = 1;

			for (unsigned int i = 0; i < B_effective->length - iteration; ++i) {
				BITS(B_effective)[i] = 0;
			}
			word_op_multiply(register_W_correct, B_effective, A);
			
			word_op_load(result_squared, register_S, 
						 (processor_size + algorithm_Z) - algorithm_m * (iteration - 1));
			word_op_multiply(result_squared, result_squared, result_squared);
			
			word_op_add(register_W_correct, result_squared, -1, 0);
			
			//word_op_rightshift(register_W_correct, 
			//				   -(algorithm_m * (algorithm_n + 1) + 2 * algorithm_Z));
			
			register_W_correct->is_signed = 0;
			buffer3 = word_makestring(result_squared, 1 << algorithm_m);
			register_W_correct->is_signed = 1;
			
			free(B_effective);
			free(result_squared);
			free(register_W_correct);
		}
		
		printf("%s\n", delimiter);
		if (iteration < B_digits[0]) {
			printf("{W} = %s\n", buffer1);
		}
		
		if (iteration >= delta) {
			printf("{W} = %s\n", buffer2);
			//printf(" ---- %s\n", buffer3);
		}
		
		printf("      (overflow = %s, underflow = %s)\n\n",
   			   register_W_practical->overflow ? "YES" : "NO", register_W_practical->underflow ? "YES" : "NO");
		
		free(buffer1);
		free(buffer2);	
		
		register_W->is_signed = 0;
		buffer2 = word_makestring(register_W, 1 << algorithm_m);
		register_W->is_signed = 1;
		
		printf("{W}t= %s\n", buffer2);
		printf("      (overflow = %s, underflow = %s)\n\n",
   			   register_W->overflow ? "YES" : "NO", register_W->underflow ? "YES" : "NO");
		
		free(buffer2);
#endif
		
		word_deallocate(S0s);
		word_deallocate(partial_product_term);
		word_deallocate(linearquadratic_term);
		word_deallocate(linearquadratic_term_practical);		
		
		// ---------------------------------
		
		// update the result register {S}
		word_op_leftshift(register_S, algorithm_m);
		word_op_load_constant(register_S, (iteration <= S_prime_digits[0] ? 
										   S_prime_digits[iteration] : 0), 0, algorithm_m);
		
		
		// ON-THE-FLY CONVERSION: PART 3
		if (1 == onthefly_select) {
			word_op_load(register_S_practical, register_S_m1, 0);

			word_op_load(register_2S, register_S_m1, 0);
		} else {
			word_op_load(register_2S, register_S_practical, 0);
		}

		if (0 == onthefly_select_m1) {
			word_op_load(register_S_m1, register_S_practical, 0);
							
			word_op_load(register_2S_m1, register_S_practical, 0);			
		} else {	
			word_op_load(register_2S_m1, register_S_m1, 0);
		}


		// Update the direct and "minus one" copies of the result register
		word_op_leftshift(register_S_practical, algorithm_m);
		word_op_leftshift(register_S_m1, algorithm_m);
		
		word_op_load(register_S_practical, onthefly_appended_digit, 0);
		word_op_load(register_S_m1, onthefly_appended_digit_m1, 0);		
		
		// Update the direct and "minus one" copies of the 2S register
		word_op_leftshift(register_2S, algorithm_m + 1);
		word_op_leftshift(register_2S_m1, algorithm_m + 1);
		
		word_op_load(register_2S, onthefly_appended_digit_t2, 0);
		word_op_load(register_2S_m1, onthefly_appended_digit_t2m1, 0);				

#if !defined(SUPPRESS_DETAILS)		
		// display updated result register -practical
		printf("{S}ac = %s\n", buffer1 = word_makestring(register_S_practical, 1 << algorithm_m));
		free(buffer1);
		// display updated result register -theoretical
		printf("{S}th = %s\n", buffer1 = word_makestring(register_S, 1 << algorithm_m));
		free(buffer1);
#endif
		
		if (iteration < iterations) {
			// update the multiplicand register {A} and display it
			word_op_leftshift(register_A, algorithm_m);

#if !defined(SUPPRESS_DETAILS)
			printf("{A} = %s\n\n", buffer1 = word_makestring(register_A, 1 << algorithm_m));
			free(buffer1);
#endif
			
		} else {
			
#if !defined(SUPPRESS_DETAILS)			
			printf("\n");
#endif
			
		}
		
		// update the P mask and cursor
		word_op_leftshift(P_mask, algorithm_m);
		P_cursor += algorithm_m;

	}
	// -------------------------------------
	// display/postprocess results
	// -------------------------------------
	if (0 == word_op_compare_constant(register_W, 0))
		printf("RESIDUAL SUCCESSFULLY ELIMINATED!\n");
	else
		printf("~~ RESIDUAL DIVERGED ~~\n");

	if (register_W->overflow || register_W->underflow)
		printf("~~ WARNING: OVERFLOW OCCURED ~~\n");
		
	
	word_op_rightshift(register_S, mb);
	if (0 == word_op_compare(register_S, S))
		printf("SQUARE ROOT CORRECTLY RECOVERED!\n");
	else {
		//printf("~~ WARNING: SQUARE ROOT INCORRECTLY RECOVERED ~~\n\n");
	
		printf("{S}final = %s\n(S = %s)\n",
			   buffer1 = word_makestring(register_S, 1 << algorithm_m),
			   buffer2 = word_makestring(S, 1 << algorithm_m));
	
		free(buffer1);
		free(buffer2);
	}
	
	word_pointer AB = create_word(processor_size << 1);
	word_op_multiply(AB, B, A);

	if (0 != word_op_compare_constant(register_W, 0)) {
		printf("\nEXTRA INFORMATION FOR TRACKING THE PROBLEM:"
			   "\nAB(calculated) = %s\nS(mathematica) = BaseForm[Sqrt[%s * %s],%d]\n", 
			   buffer1 = word_makestring(AB, 1 << algorithm_m),
			   buffer2 = word_makemathematicacode(A),
			   buffer3 = word_makemathematicacode(B), 1 << algorithm_m);
		
		free(buffer1);
		free(buffer2);
		free(buffer3);	
	}
	
	// deallocate memory
#if !defined(SUPPRESS_DETAILS)
	free(delimiter);
#endif
	word_deallocate(digit_multiplier_S);	
	word_deallocate(digit_multiplier_S_practical);	
	word_deallocate(digit_multiplier_B);	
	word_deallocate(register_W);
	word_deallocate(register_W_practical);	
	word_deallocate(register_A);
	word_deallocate(onthefly_appended_digit);			
	word_deallocate(onthefly_appended_digit_m1);
	word_deallocate(onthefly_appended_digit_t2);			
	word_deallocate(onthefly_appended_digit_t2m1);
	word_deallocate(register_2S);		
	word_deallocate(register_2S_m1);			
	word_deallocate(register_S);	
	word_deallocate(register_S_practical);	
	word_deallocate(register_S_m1);	
	word_deallocate(P_mask);
	word_deallocate(P);
	word_deallocate(Sdot);
	free(B_digits);
	free(S_prime_digits);
	word_deallocate(S_prime);
	
	word_deallocate(S);
	word_deallocate(random_seed2);	
	word_deallocate(random_seed1);	
	word_deallocate(B);		
	word_deallocate(A);	
	
	return 0;
}