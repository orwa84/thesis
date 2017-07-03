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
	const unsigned short 
	algorithm_m = 9, 
	algorithm_n = 8, 
	algorithm_Z = 11;
	
	// dependent system parameters
	const unsigned short overhead_iterations = 
	(unsigned short) ceil((double) algorithm_Z / algorithm_m);
	const unsigned short preliminary_iterations = 
	(unsigned short) floor((double) algorithm_Z / algorithm_m);
	const unsigned short excess_bits = 
	(unsigned short) (algorithm_Z % algorithm_m ? algorithm_m - algorithm_Z % algorithm_m : 0);
	
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
		   "         ORWA-AMIN MULTIPLICATIVE SQUARE ROOTING ALGORITHM SIMULATOR         \n"
		   "-----------------------------------------------------------------------------\n"
		   " Configuration: <VERY BASIC> \n"
		   " - Signed-digit result with on-the-fly conversion of digits: NO\n"
		   " - Supports rounding: NO\n"
		   " - Actual digit selection: NO (precomputed)\n"
		   " - Digit selection using an actual table: NO\n"
		   " - Carry-Save residual: NO\n"
		   " - Signed-digit multiplier B with automatic conversion: NO\n"
		   "-----------------------------------------------------------------------------\n"
		   );
	
	printf("System parameters:\n"
		   " - m: %d bits\t\t→ RADIX = %d\n - n: %d iterations\n - Z: %d bits\n"
		   "\nSystem variables:\n"
		   " - size of operands:    %d bits\n"			// "m × n ="
		   " - overhead iterations: %d iteration(s)\n"	// "⎡Z/m⎤ = "
		   " - excess result bits:  %d bit(s)\n",		// "⎡Z/m⎤m - Z = "
		   algorithm_m, 1 << algorithm_m, algorithm_n, algorithm_Z, 
		   processor_size, overhead_iterations, excess_bits);
	
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
	word_pointer S_prime = create_word(processor_size + algorithm_m * overhead_iterations);
	word_op_load(S_prime, S, excess_bits);
	
	unsigned int *S_prime_digits = word_makelist(S_prime, algorithm_m);
	
	assert(S_prime_digits[0] == (algorithm_n + overhead_iterations) && 
		   "NOT ENOUGH RESULT DIGITS PRODUCED");
	if (S_prime_digits[0] != (algorithm_n + overhead_iterations)) {
		perror("NOT ENOUGH RESULT DIGITS PRODUCED");
		return 0;
	}
	
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
	algorithm_m * (algorithm_n + (excess_bits > 0 ? 1 : 0));
	const unsigned short register_A_size = 
	algorithm_m * (2 * algorithm_n + overhead_iterations - 1);
	const unsigned short register_W_size = 
	2 * algorithm_m * (algorithm_n + 1) + 2 * algorithm_Z + excess_bits + 1;
	
	word_pointer register_S = create_word(register_S_size);
	word_pointer register_A = create_word(register_A_size);
	word_pointer register_W = create_word(register_W_size);
	
	// make the W register of th signed type, to ensure that
	// a potential case of overflow is correctly interpreted.
	register_W->is_signed = 1;
	
	word_pointer digit_multiplier_B = create_word(algorithm_m);
	word_pointer digit_multiplier_S = create_word(algorithm_m);	
	
	// -------------------------------------
	// initialization step of the algorithm
	// -------------------------------------
	
	// the S register will already have zeros in it at this
	// point {S'} = 0
	
	// {A} = A
	word_op_load(register_A, A, 0);
	
	// {W} = b1×A
	word_op_load_constant(digit_multiplier_B, B_digits[1], 0, algorithm_m);
	word_op_multiply(register_W, digit_multiplier_B, A);
	
#if !defined(SUPPRESS_DETAILS)	
	// display algorithm's status
	printf("iteration 0 (initialization):\n"
		   "{S} = %s\n"
		   "{A} = %s\n      (overflow = %s, underflow = %s)\n"
		   "{W} = %s\n      (overflow = %s, underflow = %s)\n\n",
		   buffer1 = word_makestring(register_S, 1 << algorithm_m),
		   buffer2 = word_makestring(register_A, 1 << algorithm_m),
		   register_A->overflow ? "YES" : "NO", register_A->underflow ? "YES" : "NO",
		   buffer3 = word_makestring(register_W, 1 << algorithm_m),
		   register_W->overflow ? "YES" : "NO", register_W->underflow ? "YES" : "NO");
	
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
		 iteration <= algorithm_n + overhead_iterations; ++iteration) {
		
		// update the residual register {W}
		word_op_leftshift(register_W, algorithm_m * 2);
		
		// load the next multiplier digit bi+1 into "digit_multiplier_B"
		if (iteration < B_digits[0])
			word_op_load_constant(digit_multiplier_B, B_digits[iteration + 1], 0, algorithm_m);
		else word_op_load_constant(digit_multiplier_B, 0, 0, algorithm_m);
		
		// load the current delayed root digit s'i into "digit_multiplier_S"
		word_op_load_constant(digit_multiplier_S, S_prime_digits[iteration], 0, algorithm_m);
		
#if !defined(SUPPRESS_DETAILS)		
		register_W->is_signed = 0;
		buffer1 = word_makestring(register_W, 1 << algorithm_m);
		register_W->is_signed = 1;	
		
		printf("iteration %u (", iteration);
		
		if (iteration > preliminary_iterations) {
			printf("s' = %d", S_prime_digits[iteration]);
		}
		
		if (iteration > preliminary_iterations && iteration < B_digits[0])
			printf(", ");
		
		if (iteration < B_digits[0]) {
			// maximum supported radix has 3 decimal digits per high-radix 
			// digit, plus a null character.
			buffer2 = malloc(4);
			sprintf(buffer2, "%.0f", word_approximatevalue(digit_multiplier_B));
			
			printf("b = %s", buffer2);
			
			free(buffer2);
		}
		
		printf("):\n      %s\n", buffer1);
		free(buffer1);
#endif
		
		word_pointer partial_product_term = create_word(register_W_size);
		word_pointer linearquadratic_term = create_word(register_W_size);
		
		// construct the partial product term
		word_op_multiply(partial_product_term, digit_multiplier_B, register_A);
		word_op_leftshift(partial_product_term, algorithm_m);
		
		// construct the linear-quadratic term
		word_op_load(linearquadratic_term, register_S, algorithm_m + 1);
		word_op_load_constant(linearquadratic_term, S_prime_digits[iteration], 0, algorithm_m);
		word_op_multiply(linearquadratic_term, digit_multiplier_S, linearquadratic_term);
		word_op_leftshift(linearquadratic_term, algorithm_m * (algorithm_n + 1) + 2 * algorithm_Z);
		
#if !defined(SUPPRESS_DETAILS)		
		// display both terms
		buffer1 = word_makestring(partial_product_term, 1 << algorithm_m);
		buffer2 = word_makestring(linearquadratic_term, 1 << algorithm_m);
		buffer3 = word_makestring(register_W, 1 << algorithm_m);
		
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
			
			if (iteration > preliminary_iterations) {
				buffer4[1] = '-';
				printf(buffer4, word_cleanstring(buffer2));
			}
		}
		
		free(buffer1);
		free(buffer2);
		free(buffer3);				
#endif
		
		// now use both terms to update the residual word
		word_op_add(register_W, partial_product_term, +1, 0);
		
#if !defined(SUPPRESS_DETAILS)		
		register_W->is_signed = 0;
		buffer1 = word_makestring(register_W, 1 << algorithm_m);
		register_W->is_signed = 1;				
#endif
		
		word_op_add(register_W, linearquadratic_term, -1, 0);
		
#if !defined(SUPPRESS_DETAILS)		
		register_W->is_signed = 0;
		buffer2 = word_makestring(register_W, 1 << algorithm_m);
		register_W->is_signed = 1;
		
		printf("%s\n", delimiter);
		if (iteration < B_digits[0]) {
			printf("{W} = %s\n", buffer1);
		}
		
		if (iteration > preliminary_iterations) {
			printf("{W} = %s\n", buffer2);
		}
		
		printf("      (overflow = %s, underflow = %s)\n\n",
   			   register_W->overflow ? "YES" : "NO", register_W->underflow ? "YES" : "NO");
		
		free(buffer1);
		free(buffer2);		
#endif
		
		word_deallocate(partial_product_term);
		word_deallocate(linearquadratic_term);
		
		// ---------------------------------
		
		// update the result register {S}
		word_op_leftshift(register_S, algorithm_m);
		word_op_load_constant(register_S, S_prime_digits[iteration], 0, algorithm_m);
		
#if !defined(SUPPRESS_DETAILS)		
		// display updated result register
		printf("{S} = %s\n", buffer1 = word_makestring(register_S, 1 << algorithm_m));
		free(buffer1);
#endif
		
		if (iteration < (algorithm_n + overhead_iterations)) {
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
	
	
	word_op_rightshift(register_S, excess_bits);
	if (0 == word_op_compare(register_S, S))
		printf("SQUARE ROOT CORRECTLY RECOVERED!\n");
	else {
		printf("~~ WARNING: SQUARE ROOT INCORRECTLY RECOVERED ~~\n\n");
		
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
	word_deallocate(digit_multiplier_B);	
	word_deallocate(register_W);
	word_deallocate(register_A);
	word_deallocate(register_S);	
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