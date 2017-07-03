/*
 *  word.h
 *  mechanical project
 *
 *  Created by Orwa on 1/23/12.
 *
 */

//
// --- d o c u m e n t a t i o n --- s t a r t s --- h e r e
//
// BASIC CODE CONCEPTS:
//
// "word":
//		is a program memory structure, variable in size, that contains the raw contents of a hardware 
//		register, as individual bits plus some additional information in the form of a header.
//
//
// TECHNICAL DETAILS:
//
//  - storage:
//		a word structure is stored as a header followed by a variable number of bytes, each containing
//		either "1" or "0", using a little-endian order (less significant bits stored first, or in lower
//		addresses). note that the choice of a little-endian ordering is intended to facilitate alignment
//		as well as the manual implementation of additions/subtractions.
//
//  - C definition:
//		a word structure is defined as a pointer to the word_header structure, while making it implicitly
//		understood that the header structure will be followed by a variable number of bytes, containing
//		the value of individual bits in the word structure (also called the bit array). hence, a word 
//		structure is composed of a header and a bit array.
//
//		the length of the bit array, which represents the width of the word, is stored in the header in
//		the field named "length", which is of the type "unsigned short".
//
//  - size:
//		the size of the overall structure is equal to the size of the header "sizeof(word_header)", 
//		plus the size of the bit array, which is equal to "word->length * sizeof(unsigned char)".
//
//  - accessing the bits:
//		to access the bits of a word structure, we need to advance the pointer to skip the header, then
//		to advance it just enough to address the target bit of the bit array, while explicitly casting
//		the pointer type to "unsigned char*".
//
//		the shortest way to do this in the C programming language is using the line:
//			((unsigned char*) (word + 1))[bit_index]
//
//		which is shorter (though less clear) than the more explicit form:
//			((unsigned char*) ((void*) word + sizeof(struct word_header)))[bit_index]
//
//		yet in either case, it seems that the frequency of this operation necessitates that there be
//		an even shorter (and less error prone) method of accessing the bits, which is why a dedicated
//		macro named "BITS" was written for this purpose.
//
//		--- d o c u m e n t a t i o n --- b r e a k s --- h e r e ---
//

// this macro provides an easy way to access the bit array
// following the header of a "word" structure.
#define BITS(word)	((unsigned char*) ((void*) word + sizeof(struct word_header)))

//
//		--- d o c u m e n t a t i o n --- r e s u m e s --- h e r e ---
//
//  - data integrity:
//		of central importance to this code is the ability to track the integrity of the data stored
//		in hardware registers (modelled by "word" structures), which corresponds to ensuring that no
//		"meaningful" digits or data is being lost by applying a sequence of binary and mathematical
//		operations on registers.
//
//		to address this requirement, additional two fields were added to the word_header structure
//		which are the flags named "overflow" and "underflow".
//
//		interestingly, what signifies an underflow or an overflow is largely determined by how the
//		bit data is interpreted (e.g. polarity, sign coding, etc.).
//
//		in the case if this code, a positive polarity is assumed throughout, meaning that a 0-valued
//		bit corresponds to a binary digit with the value zero, whereas a 1-valued bit corresponds to
//		a binary digit with the vale one.
//
//		-- underflow:
//			since a positive polarity is assumed, an underflow can only occur from a right shift if
//			it resulted in a one or more 1-valued bits being shifted out.
//		-- overflow:
//			as for overflow, on the other hand, the case is a little bit complicated as it also 
//			depends on the sign coding of the quantity stored in the register. for example, if the 
//			quantity was unsigned then, similar to the case of an underflow, a left shift for example
//			can only result in an overflow if it caused a 1-valued but to be shifted out, but what 
//			if the quantity was signed?
//
//			for example, shifting minus one "11...111" two bit positions to the left will result in
//			"11...100", which is minus four. in this case, two ones were shifted out but no overflow
//			occured. however, if the same data was interpreted as an unsigned quantity then it would
//			signify a case of an overflow, so obviously the way in which we determine an overflow 
//			depends on whether the register type is signed or unsigned.
//
//			for this reason, an extra flag named "is_signed" was added to the word_header structure,
//			which helps determining a case of an overflow. 
//
//			in the case of a signed register, a left shift is only associated with an overflow if it
//			caused a value other than that of the "sign bit" to be shifted out.
//
// --- d o c u m e n t a t i o n --- e n d s --- h e r e
//

typedef struct word_header {
	// length of the word in binary bits, helps determining the 
	// size of the overall structure (header + individual bits).
	unsigned short length;
	// a flag indicating whether an overflow occured from either
	// a left shift or an addition.
	unsigned char overflow;	
	// a flag indicating whether an underflow occured from a 
	// right shift.
	unsigned char underflow;
	// a flag indicating whether the quantity stored in the
	// word bits is signed (helps determining an overflow).
	unsigned char is_signed;
} *word_pointer;


// --------------------------------------------------
// create_word
// --------------------------------------------------
struct word_header* create_word(unsigned short length) {
	
	// calculate the number of bytes needed to store the whole
	// structure, which includes both the header and the bit
	// array.
	const size_t byte_count = sizeof(struct word_header) + 
	length * sizeof(unsigned char);
	struct word_header* word = NULL;
	
	// allocate this number of bytes.
	word = malloc(byte_count);
	
	assert(NULL != word);
	if (NULL == word) {
		perror("Couldn't allocate memory for a word.");
		return NULL;
	}
	
	// initialize the fields properly, and reset bit array
	// to zero.
	memset((void*) word, 0, byte_count);
	word->length = length;
	
	// note that default word type is "unsigned"
	
	return word;
}

// --------------------------------------------------
// create_word_duplicate
// --------------------------------------------------
struct word_header* create_word_duplicate(struct word_header* source) {
	
	assert(NULL != source && source->length > 0);
	if (NULL == source || 0 == source->length) {
		perror("Invalid source word passed to create_word_duplicate.");
		return NULL;
	}
	
	struct word_header* result = create_word(source->length);
	if (NULL == result)
		return NULL;
	
	const size_t byte_count = sizeof(struct word_header) + 
	sizeof(unsigned char) * source->length;
	memcpy((void*) result, (void*) source, byte_count);
	
	return result;
}

// --------------------------------------------------
// word_deallocate
// --------------------------------------------------
void word_deallocate(struct word_header* word) {
	
	assert(NULL != word);
	if (NULL == word) {
		perror("NULL pointer passed to word_deallocate.");
		return;
	}
	
	free((void*) word);
}

// --------------------------------------------------
// word_randomize
// --------------------------------------------------
//   stores a random, yet-normalized value in the bits
//   of the structure linked by "word".
//
// notes:
// - to result in a different sequence of random values
//   each time the program is executed, the function
//   "initialize_randomizer" should be called at least
//   once prior to calling this function.
// --------------------------------------------------
void word_randomize(struct word_header* word) {
	
	assert(NULL != word && word->length > 0);
	if (NULL == word || 0 == word->length) {
		perror("Invalid word passed to word_randomize.");
		return ;
	}
	
	// for an unsigned word, the most-significant bit is one
	// for a signed word, on the other hand, the most-signi-
	// ficant bit (the sign bit) is random whereas the second
	// bit is opposite to the sign bit.
	unsigned char previous_bit;
	do previous_bit = random_bit();
	while (random_bit() == previous_bit);
	
	// following the previous loop we'll have a random sequence
	// of two different bit values (01 or 10), where previous_
	// value is the first of these two bits.
	
	int i = word->length - 1;
	if (word->is_signed) {
		BITS(word)[i--] = previous_bit;
		BITS(word)[i--] = (previous_bit ? 0 : 1);
	} else {
		BITS(word)[i--] = 1;
		
		if (previous_bit)
			BITS(word)[i--] = 0;
	}

	while (i >= 0)
		BITS(word)[i--] = random_bit();
}


// --------------------------------------------------
// word_op_negate
// --------------------------------------------------
void word_op_negate(struct word_header* word) {
	
	assert(NULL != word && word->length > 0);
	if (NULL == word || 0 == word->length) {
		perror("Invalid word passed to word_op_negate.");
		return ;
	}
	assert(word->is_signed);
	if (!word->is_signed) {
		perror("an unsigned word cannot be negated (word_op_negate).");
		return ;
	}
	
	// compute the sign bit, but don't alter it yet
	unsigned char final_sign = (BITS(word)[word->length - 1] ? 0 : 1);

	// to negate a signed number we invert all the bits
	// and then add one.
	int sum = 1;
	for (unsigned int i = 0; i < word->length; ++i) {
		
		// remember that the most-significant bit of the
		// result is the sign bit, and hence should not
		// be involved in the inversion/addition process.
		// also remember that the word should either be
		// padded with 0's or 1's depending on the original 
		// sign (positive or minus, respectively).
		unsigned char source_bit = BITS(word)[word->length - 1];
		if (i < word->length)
			source_bit = BITS(word)[i];
		
		// add the initial 1-valued sum to the inverted
		// word bits
		sum += (source_bit ? 0 : 1);
		
		if (i == word->length - 1 && (sum & 1) != final_sign) {
			word->overflow = 1;
		}

		if (i < word->length - 1)
			BITS(word)[i] = sum & 1;
		else if (i == word->length - 1)
			BITS(word)[i] = final_sign;
		
		sum >>= 1;
	}
}

// --------------------------------------------------
// word_op_abs
// --------------------------------------------------
//
// warning:
// - the word returned by this function call is
//   dynamically allocated using "create_word" and
//   hence should be freed manually using "word_deall
//   -ocate" to avoid memory leaks.
// --------------------------------------------------
struct word_header* word_op_abs(struct word_header* word) {
	
	assert(NULL != word && word->length > 0);
	if (NULL == word || 0 == word->length) {
		perror("Invalid word passed to word_op_abs.");
		return NULL;
	}
	
	struct word_header* result = create_word_duplicate(word);
	
	// if the word was intended to convey a signed amount and
	// had a sign bit of one (negative), then invert and add
	// one to get the amplitude or the absolute value.
	if (result->is_signed && 1 == BITS(result)[result->length - 1]) {
		BITS(result)[result->length - 1] = 0;
		
		int sum = 1;
		for (unsigned int i = 0; i < result->length; ++i) {

			// remember that the most-significant bit of the
			// result is the sign bit, and hence should not
			// be involved in the inversion/addition process
			// also remember that the word here is negative
			// and hence should be padded with 1's
			unsigned char source_bit = 1;
			if (i < result->length - 1)
				source_bit = BITS(result)[i];

			// add the initial 1-valued sum to the inverted
			// word bits
			sum += (source_bit ? 0 : 1);
			
			BITS(result)[i] = sum & 1;
			sum >>= 1;
		}
	}
	
	return result;
}

// --------------------------------------------------
// word_makelist
// --------------------------------------------------
//   returns a list of digits to the radix "2^bits_per_
//   digit" in a big-endian ordering (most-significant
//   digits in lower addresses), which is more convenient
//   for display purposes.
//
// notes:
// - the first element of the returned list is not an 
//   actual digit, rather, it is used to communicate
//   the total number of digits in the list.
// - hence, to access the "i"'th digit in the list we
//   need something like: list[list[0] - i] or to 
//   traverse over all the digits from the most-sig
//   nificant to the least-significant we need some
//   thing like: for (i = 1; i <= list[0]; ++i) where
//   "i" can be served directly as an index.
//   and hence srves the role of a header.
// - this function is used inernally by the functions
//   "word_makestring" and "word_makemathematicacode".
//
// warning:
// - the list returned by this function call is 
//   dynamically allocated using "malloc" and hence 
//   should be freed manually to avoid memory leaks.
// --------------------------------------------------
unsigned int* word_makelist(struct word_header* word, unsigned char bits_per_digit) {
	
	assert(NULL != word && word->length > 0);
	if (NULL == word || 0 == word->length) {
		perror("Invalid word passed to word_makelist.");
		return NULL;
	}
	
	assert(bits_per_digit > 0);
	if (bits_per_digit == 0) {
		perror("Invalid radix passed to word_makelist.");
		return NULL;
	}
	
	// compute the number of "bits_per_digit" blocks in
	// the number, this is the number of digits to be
	// returned in the list.
	const unsigned int digit_count = (unsigned int)
		ceil((double) word->length / bits_per_digit);
	// allocate an array of the type "unsigned int" to
	// hold the digit values, with an additional header
	// element dedicated to the number of digits in the
	// list.
	unsigned int* storage = 
		malloc(sizeof(unsigned int) * (digit_count + 1));
	
	assert(NULL != storage);
	if (NULL == storage) {
		perror("Couldn't allocate memory for word_makelist.");
		return NULL;
	}
	
	// store the number of digits in the additional
	// header element.
	storage[0] = digit_count;
	
	// to make a digit list we need to work on the amplitude
	// (the absolute value) stored in the number.
	struct word_header* amplitude = word_op_abs(word);
	
	// "i" is a digit counter varying from "0" (least-
	// significant digit) to "digit_count - 1" (most-
	// significant digit of "word").
	// "j" is a pointer to the corresponding item of
	// final list. To result in a big-endian ordering
	// for the digit list, "j" is initialized with the
	// address of the last list item and is decremented
	// per iteration.
	unsigned int* j = storage + digit_count; // last digit
	for (unsigned int i = 0; i < digit_count; ++i, --j) {
		
		// here, we calculate the value of the i'th digit
		unsigned int digit_value = BITS(amplitude)[i * bits_per_digit];
		for (unsigned int k = 1; k < bits_per_digit; ++k) {
			if ((i * bits_per_digit + k) >= amplitude->length)
				break;
			
			digit_value += (BITS(amplitude)[i * bits_per_digit + k] << k);
		}
		
		// assign the digit value to the corresponding
		// item of the list.
		*j = digit_value;
	}
	
	word_deallocate(amplitude);
	
	// return a pointer to the list.
	return storage;
}

// --------------------------------------------------
// word_sign
// --------------------------------------------------
char word_sign(struct word_header* word) {
	return (!word->is_signed || 0 == BITS(word)[word->length - 1] ? ' ' : '-');
}

// --------------------------------------------------
// word_makestring
// --------------------------------------------------
//   returns a textual representation of the word bits
//   expressed to the base 2, 4, 8 or 16.
//
// warning:
// - the string returned by this function call is 
//   dynamically allocated using "malloc" and hence 
//   should be freed manually to avoid memory leaks.
// --------------------------------------------------
char* word_makestring(struct word_header* word, unsigned short base) {
	
	assert(NULL != word && word->length > 0);
	if (NULL == word || 0 == word->length) {
		perror("Invalid word passed to word_makestring.");
		return NULL;
	}
	
	assert(2 == base || 4 == base || 8 == base || 
		   16 == base || 32 == base || 64 == base || 
		   128 == base || 256 == base || 512 == base);
	switch (base) {
		case 2: case 4: case 8: 
		case 16: case 32: case 64: 
		case 128: case 256: case 512:
			break;
		default:
			perror("Invalid base passed to word_makestring.");
			return NULL;
	}
	
	// compute the correct value for bits_per_digit of
	// word_makelist.
	unsigned char bits_per_digit = 0;
	while (base >>= 1) {
		++bits_per_digit;
	}
	
	// get a list of digits.
	unsigned int* digits = word_makelist(word, bits_per_digit);
	
	if (NULL == digits)
		return NULL;
	
	// look up the number of characters per digit, 
	// depending on the radix.
	unsigned char characters_per_digit = 1;
	if (bits_per_digit >= 7) {  // base = 128 and above {128, 256, 512}
		characters_per_digit = 4;
	} else if (bits_per_digit >= 5) { // base = 32 and above {32, 64}
		characters_per_digit = 3;
	}
	
	// use this number to allocate memory for the
	// resulting string, note that an additional
	// byte is needed for the null character.
	const unsigned int digit_count = digits[0];

	char* storage = malloc(digit_count * characters_per_digit + 1);
	
	assert(NULL != storage);
	if (NULL == storage) {
		perror("Couldn't allocate memory for word_makestring.");
		// so that a case of error will not result
		// in a memory leak.
		free(digits);
		return NULL;
	}
	
	// terminate the resulting string.
	storage[digit_count] = '\0';
	
	// produce the string by translating digit values
	// to either single digit characters (for radices
	// less or equal to 16) or decimal values (higher
	// radices).
	for (unsigned int i = 0; i < digit_count; ++i) {
		if (bits_per_digit <= 4) {
			if ((digits+1)[i] < 10)
				storage[i] = (char) '0' + (digits+1)[i];
			else storage[i] = (char) 'A' + ((digits+1)[i] - 10);
			
		} else {
			char format_string[5];
			// remember that characters_per_digit includes the one character
			// needed for separating the digits '|'
			snprintf(format_string, 5, "%%0%du", characters_per_digit - 1);
			sprintf(storage + i * characters_per_digit, format_string, (digits+1)[i]);
			storage[(i + 1) * characters_per_digit - 1] = '|';
		}
	}
	
	// all non-single-character-digit cases {base larger than 16}
	if (bits_per_digit > 4) { 
		storage[digit_count * characters_per_digit - 1] = '\0';
	}
	
	// now that the digit list has been fully translated
	// into a string, we can free the space associated
	// with the list.
	free(digits);
	
	return storage;
}

// --------------------------------------------------
// word_cleanstring
// --------------------------------------------------

char* word_cleanstring(char* string) {
	
	for (unsigned int i = 0, j = 0; i < strlen(string); ++i) {
		if (string[i] != '0' && string[i] != '|' && !j) j = 1;
		if ((string[i] == '0' || string[i] == '|') && !j) string[i] = ' ';
	}
	for (int i = strlen(string) - 1, j = 0; i >= 0; --i) {
		if (string[i] != '0' && string[i] != '|' && !j) j = 1;
		if ((string[i] == '0' || string[i] == '|') && !j) string[i] = ' ';
	}
	
	return string;
}

// --------------------------------------------------
// word_makemathematicacode
// --------------------------------------------------
//   returns the code line needed to evaluate the de-
//   cimal equiavlent of the word bits through mathe-
//   matica, which will allow us to verify the comp-
//   utations performed by this code.
//
// warning:
// - the string returned by this function call is 
//   dynamically allocated using "malloc" and hence 
//   should be freed manually to avoid memory leaks.
// --------------------------------------------------
char* word_makemathematicacode(struct word_header* word) {
	
	assert(NULL != word && word->length > 0);
	if (NULL == word || 0 == word->length) {
		perror("Invalid word passed to word_makemathematicacode.");
		return NULL;
	}
	
	// retrieve the number as a list of digits to the base 512.
	unsigned int* digits_to_the_base_512 = word_makelist(word, 9);
	if (NULL == digits_to_the_base_512) {
		return NULL;
	}
	
	// allocate a number of bytes sufficient for the mathematica
	// code line which is equal to "FromDigits[{D1, D2, ...}, 512]"
	const size_t bytes_count = 20 + (digits_to_the_base_512[0] * 5) - 1;
	
	char* storage = malloc(bytes_count);
	assert(NULL != storage);
	if (NULL == storage) {
		perror("Couldn;t allocate memory for word_makemathematicacode.");
		free(digits_to_the_base_512);
		return NULL;
	}
	
	char* cursor = storage;
	memset((void*) storage, 0, bytes_count);
	
	// construct the code line while keeping track of remaining
	// bytes in the storage buffer to avoid write violations.
	int remaining_size = bytes_count;
	
	if (word->is_signed && 1 == BITS(word)[word->length - 1]) {
		*(cursor++) = '-';
		--remaining_size;
	}
	
	snprintf(cursor, remaining_size, "FromDigits[{");
	
	// note that the "sizeof" operator returns the size in bits
	// INCLUDING the null character, which is why we have to
	// subtract one.
	remaining_size -= sizeof("FromDigits[{") - 1;
	cursor += sizeof("FromDigits[{") - 1;
	
	for (unsigned int i = 0; i < digits_to_the_base_512[0]; ++i) {
		snprintf(cursor, remaining_size, "%d", (digits_to_the_base_512+1)[i]);
		
		while ('\0' != *cursor) {
			++cursor;
			--remaining_size;
		}
		
		if (i != digits_to_the_base_512[0] - 1) {
			strncpy(cursor, ", ", remaining_size - 1);
			
			cursor += sizeof(", ") - 1;
			remaining_size -= sizeof(", ") - 1;
		}
	}
	
	// since all the digits were consumed, we can now release
	// the space associated with the digit list.
	free(digits_to_the_base_512);
	
	strncpy(cursor, "}, 512]", remaining_size - 1);
	
	cursor += sizeof("}, 512]") - 1;
	remaining_size -= sizeof("}, 512]") - 1;
	
	assert(remaining_size >= 1 && '\0' == *cursor);
	if (remaining_size < 1 || '\0' != *cursor) {
		perror("Insufficient space for mathematica code, revision of code is needed.");
		free(storage);
		return NULL;
	}
	
	return storage;
}

// --------------------------------------------------
// word_op_compare
// --------------------------------------------------
char word_op_compare(struct word_header* word1, struct word_header* word2) {
	
	assert(NULL != word1 && word1->length > 0 && NULL != word2 && word2->length > 0);
	if (NULL == word1 || 0 == word1->length || NULL == word2 || 0 == word2->length) {
		perror("Invalid word was passed to word_op_compare.");
		return 2;
	}
	
	// in the case of signed numbers, we only need to check any mismatch in sign
	// if the sign of both numbers is the same, then no need to change the code
	unsigned char sign1 = 0, sign2 = 0;
	if (!word1->is_signed || 0 == BITS(word1)[word1->length - 1])
		sign1 = 1;
	if (!word2->is_signed || 0 == BITS(word2)[word2->length - 1])
		sign2 = 1;
	
	if (sign1 > sign2) 
		return +1;
	else if (sign1 < sign2)
		return -1;
	
	
	const unsigned int size = (word1->length > word2->length ? 
							   word1->length : word2->length);
	
	for (int i = size - 1; i >= 0; --i) {
		
		unsigned char source_bit1 = (sign1 ? 0 : 1), source_bit2 = (sign2 ? 0 : 1);
		if (i < word1->length) source_bit1 = BITS(word1)[i];
		if (i < word2->length) source_bit2 = BITS(word2)[i];
		
		if (source_bit1 > source_bit2) return +1;
		if (source_bit1 < source_bit2) return -1;
	}
	
	return 0;
}

// --------------------------------------------------
// word_op_compare_constant
// --------------------------------------------------
char word_op_compare_constant(struct word_header* word, int constant) {
	
	assert(NULL != word && word->length > 0);
	if (NULL == word || 0 == word->length) {
		perror("Invalid word was passed to word_op_compare_constant.");
		return 2;
	}
	
	// in the case of signed numbers, we only need to check any mismatch in sign
	// if the sign of both numbers is the same, then no need to change the code
	unsigned char sign1 = 0, sign2 = 0;
	if (!word->is_signed || 0 == BITS(word)[word->length - 1])
		sign1 = 1;
	if (constant >= 0)
		sign2 = 1;
	
	if (sign1 > sign2) 
		return +1;
	else if (sign1 < sign2)
		return -1;
	
	const unsigned int size = (word->length > 8 * sizeof(unsigned int) ? 
							   word->length : 8 * sizeof(unsigned int));
	
	for (int i = size - 1; i >= 0; --i) {
		
		unsigned char source_bit1 = (sign1 ? 0 : 1), source_bit2 = (sign2 ? 0 : 1);
		if (i < word->length) 
			source_bit1 = BITS(word)[i];
		if (i < 8 * sizeof(unsigned int)) 
			source_bit2 = (unsigned char) ((unsigned int) 1 & (constant >> i));
		
		if (source_bit1 > source_bit2) return +1;
		if (source_bit1 < source_bit2) return -1;
	}
	
	return 0;
}

// --------------------------------------------------
// word_op_rightshift
// --------------------------------------------------
void word_op_rightshift(struct word_header* word, unsigned short bitcount) {
	
	assert(NULL != word && word->length > 0);
	if (NULL == word || 0 == word->length) {
		perror("Invalid word was passed to word_op_rightshift.");
		return ;
	}
	
	// extension bit is determined by the sign variable which is
	// equal to zero in the case of an unsigned word and equal
	// to the sign bit in the case of a signed word.
	// in effect, the choice between a logical and an arithmetic
	// right shift is determined by whether the shifted word is
	// of a signed or an unsigned type.
	unsigned char sign = 0;
	if (word->is_signed)
		sign = BITS(word)[word->length - 1];
	
	for (unsigned int i = 0; i < word->length; ++i) {
		
		unsigned int source_bit = sign;
		if ((i + bitcount) < word->length)
			source_bit = BITS(word)[i + bitcount];
		
		if (i < bitcount && BITS(word)[i] != 0)
			word->underflow = 1;
		
		BITS(word)[i] = source_bit;
	}
}

// --------------------------------------------------
// word_op_leftshift
// --------------------------------------------------
void word_op_leftshift(struct word_header* word, unsigned short bitcount) {
	
	assert(NULL != word && word->length > 0);
	if (NULL == word || 0 == word->length) {
		perror("Invalid word was passed to word_op_leftshift.");
		return ;
	}
	
	unsigned char sign = 0, sign_bit = 0;
	if (word->is_signed) {
		sign_bit = 1;
		sign = BITS(word)[word->length - 1];
	}
	
	for (int i = word->length - 1 - sign_bit; i >= 0; --i) {
		
		unsigned int source_bit = 0;
		if ((i - bitcount) >= 0)
			source_bit = BITS(word)[i - bitcount];
		
		if (((word->length - 1 - sign_bit) - i) < bitcount && BITS(word)[i] != sign)
			word->overflow = 1;
		
		BITS(word)[i] = source_bit;
	}
}

// --------------------------------------------------
// word_op_bitinvert
// --------------------------------------------------
void word_op_bitinvert(struct word_header* word) {
	
	assert(NULL != word && word->length > 0);
	if (NULL == word || 0 == word->length) {
		perror("Invalid word was passed to word_op_bitinvert.");
		return ;
	}
	
	// bitinvert is unaffected by whether the word is signed
	// or not, in either case it simply inverts all the bits
	
	for (unsigned int i = 0; i < word->length; ++i)
		BITS(word)[i] = (BITS(word)[i] ? 0 : 1);
}

// --------------------------------------------------
// word_approximatevalue
// --------------------------------------------------
double word_approximatevalue(struct word_header* word) {

	assert(NULL != word && word->length > 0);
	if (NULL == word || 0 == word->length) {
		perror("Invalid word was passed to word_approximatevalue.");
		return 0.0;
	}
	
	double approximation = 0.0;
	for (int i = word->length - 1, first_iteration = 1; i >= 0; --i, first_iteration = 0)
		approximation += 
			(first_iteration && word->is_signed && BITS(word)[i] != 0 ? -1.0 : 1.0) * 
			(BITS(word)[i] ? (double) ldexp(1.0, i) /* 2 to the power i */ : 0.0);
	
	return approximation;
}

// --------------------------------------------------
// word_op_extract
// --------------------------------------------------
void word_op_extract(struct word_header* word, 
					 struct word_header* sample,
					 short position) {
	
	assert(NULL != word && word->length > 0 && NULL != sample && sample->length > 0);
	if (NULL == word || 0 == word->length || NULL == sample || 0 == sample->length) {
		perror("Invalid word was passed to word_op_extract.");
		return ;
	}
	
	for (int i = position, j = 0; j < sample->length; ++i, ++j) {

		unsigned char sign = 0, source = 0;
		if (word->is_signed && BITS(word)[word->length - 1]) sign = 1;
		
		if (i >= 0 && i < word->length)
			source = BITS(word)[i];
		else if (i >= word->length)
			source = sign;
		
		BITS(sample)[j] = source;
	}
	
	return ;
}

// --------------------------------------------------
// word_op_load
// --------------------------------------------------
void word_op_load(struct word_header* word, struct word_header* value,
				  unsigned short position) {
	
	assert(NULL != word && word->length > 0 && NULL != value && value->length > 0);
	if (NULL == word || 0 == word->length || NULL == value || 0 == value->length) {
		perror("Invalid word was passed to word_op_load.");
		return ;
	}
	
	unsigned char sign1 = 0, sign2 = 0;
	if (word->is_signed  && BITS(word) [word->length  - 1]) sign1 = 1;
	if (value->is_signed && BITS(value)[value->length - 1]) sign2 = 1;	
	
	assert(sign1 == sign2);
	if (sign1 != sign2) {
		perror("loaded_word differs in sign from the base word (word_op_load).");
		return ;
	}
	
	for (unsigned int i = position, j = 0; i < position + value->length; ++i, ++j) {
		
		if (i >= word->length && BITS(value)[j] != sign1) {
			word->overflow = 1;
			return ;
		}
		
		if (i < word->length)
			BITS(word)[i] = BITS(value)[j];
	}
}

// --------------------------------------------------
// word_op_load_constant
// --------------------------------------------------
void word_op_load_constant(struct word_header* word, unsigned int value,
						   unsigned short position, unsigned short bitcount) {
	
	assert(NULL != word && word->length > 0);
	if (NULL == word || 0 == word->length) {
		perror("Invalid word was passed to word_op_load_constant.");
		return ;
	}
	assert(bitcount <= 8 * sizeof(unsigned int));
	if (bitcount > 8 * sizeof(unsigned int)) {
		perror("Invalid bitcount was passed to word_op_load_constant.");
		return ;
	}
	
	unsigned char sign = 0;
	if (word->is_signed && ((signed int) value) < 0)
		sign = 1;
	
	for (unsigned int i = position; i < position + bitcount; ++i) {
		
		if (i >= word->length && (1 & value) != sign) {
			word->overflow = 1;
			return ;
		}
		
		if (i < word->length)
			BITS(word)[i] = 1 & value;
		
		value >>= 1;
	}
}

// --------------------------------------------------
// word_op_add
// --------------------------------------------------
void word_op_add(struct word_header* result, 
				 struct word_header* added_value,
				 char sign, unsigned short position) {
	
	assert(NULL != result && result->length > 0 && NULL != added_value && added_value->length > 0);
	if (NULL == result || 0 == result->length || NULL == added_value || 0 == added_value->length) {
		perror("Invalid word was passed to word_op_add.");
		return ;
	}
	
	assert(1 == sign || -1 == sign);
	if (sign != 1 && sign != -1) {
		perror("Inavlid value passed to sign of word_op_add.");
		return ;
	}
	
	unsigned char result_sign = 0, added_value_sign = 0;
	if (result->is_signed && BITS(result)[result->length - 1])
		result_sign = 1;
	if (added_value->is_signed && BITS(added_value)[added_value->length - 1])
		added_value_sign = 1;	
	
	int sum = 0;
	
	for (unsigned int i = 0, j = position; 
		 i < added_value->length || sum != 0; ++i, ++j) {
		
		if (i >= added_value->length && j >= result->length && (sum == 0 || sum == ~0))
			break;
		
		unsigned char source = result_sign, added_bit = added_value_sign;
		if (j < result->length)
			source = BITS(result)[j];
		if (i < added_value->length)
			added_bit = BITS(added_value)[i];
		
		if (+1 == sign)
			sum += source + added_bit;
		if (-1 == sign)
			sum += source - added_bit;
		
		if (j < result->length)
			BITS(result)[j] = sum & 1;
		
		sum >>= 1;
		
		if (j >= result->length - 1 && sum != 0 && sum != ~0 && 
			(!result_sign || sum != 1 << ((result->length - 1) - j))) {
			
			result->overflow = 1;
			return ;
		}
	}
	
	return ;
}

// --------------------------------------------------
// word_op_multiply
// --------------------------------------------------
void word_op_multiply(struct word_header* result, 
					  struct word_header* multiplier, 
					  struct word_header* multiplicand) {
	
	assert(NULL != result && result->length > 0);
	if (NULL == result || 0 == result->length) {
		perror("Invalid result pointer was passed to word_op_multiply.");
		return ;
	}
	assert(NULL != multiplier && multiplier->length > 0);
	if (NULL == multiplier || 0 == multiplier->length) {
		perror("Invalid multiplier pointer was passed to word_op_multiply.");
		return ;
	}
	assert(NULL != multiplicand && multiplicand->length > 0);
	if (NULL == multiplicand || 0 == multiplicand->length) {
		perror("Invalid multiplicand pointer was passed to word_op_multiply.");
		return ;
	}	
	
	char sign1 = 0, sign2 = 0;
	if (multiplier->is_signed && BITS(multiplier)[multiplier->length - 1]) 
		sign1 = 1;
	if (multiplicand->is_signed && BITS(multiplicand)[multiplicand->length - 1])
		sign2 = 1;
	
	assert(sign1 == sign2 || result->is_signed);
	if (sign1 != sign2 && !result->is_signed) {
		perror("word_op_multiply cannot store a negative result in an unsigned result word.");
		return ;
	}
	const unsigned short na = multiplier->length,
	                     nb = multiplicand->length;
	
	unsigned int sum = 0;
	
	// copying operands provides security against the case
	// when the result is stored into one of the operands,
	// which is a common scenario (A = A * B).
	
	// copying operands is provided in a single step along
	// with finding the absolute value (in case we're dealing
	// with signed operands).
	struct word_header *multiplier_copy = word_op_abs(multiplier),
	                   *multiplicand_copy = word_op_abs(multiplicand);
	
	for (unsigned int i = 0; i < (na + nb - 1) || sum != 0; ++i) {
		
		unsigned short alpha = (i < nb ? 0 : i - nb + 1);
		unsigned short beta  = (i < nb ? i : nb - 1);
		
		unsigned int no_of_bits = 0;
		if (i < nb && i < na) 
			no_of_bits = i + 1;
		if (i < nb && i >= na) 
			no_of_bits = na;
		if (i >= nb && i < na) 
			no_of_bits = nb;
		if (i >= nb && i >= na && i < (na + nb - 1))
			no_of_bits = (na + nb - 1) - i;
		
		for (unsigned int j = 0; j < no_of_bits; ++j, ++alpha, --beta)
			sum += (BITS(multiplier_copy)[alpha] ? BITS(multiplicand_copy)[beta] : 0);
		
		if (result->is_signed && i >= result->length - 1 && sum != 0)
			result->overflow = 1;
		
		if (i >= result->length && sum != 0) {
			result->overflow = 1;
			break;
		}
		
		if (i < result->length)
			BITS(result)[i] = sum & 1;
		
		sum >>= 1;
	}
	
	free(multiplier_copy);
	free(multiplicand_copy);
	
	if (sign1 != sign2)
		word_op_negate(result);
	
	return ;
}

// --------------------------------------------------
// word_op_normalize
// --------------------------------------------------
void word_op_normalize(struct word_header* word, unsigned short change_size) {
	
	assert(NULL != word && word->length > 0);
	if (NULL == word || 0 == word->length) {
		perror("Invalid word was passed to word_op_normalize.");
		return ;
	}
	
	int i = word->length - 1;
	for (; i >= 0; --i)
		if (BITS(word)[i] != 0) break;
	
	// A zero word doesn't need normalization
	if (-1 == i) return ;
	
	if (0 != change_size) {
		const size_t new_size = sizeof(struct word_header) + sizeof(unsigned char) * i;
		realloc(word, new_size);
		
		word->length = (unsigned short) i;
	} else {
		for (int j = word->length - 1; j >= 0; --j)
			BITS(word)[j] = (i >= 0 ? BITS(word)[i--] : 0);
	}
}

// --------------------------------------------------
// word_op_isqrt
// --------------------------------------------------
//
// ADAPTED FROM WIKIPEDIA's ALGORITHM:
//
// short isqrt(short num) {
//
//  	short result = 0;
//  	short bit = 1 << 14; // The second-to-top bit is set: 1L<<30 for long
//
//  	// "bit" starts at the highest power of four <= the argument.
//  	while (bit > num)
//  		bit >>= 2;
//
//  	while (bit != 0) {
//  		if (num >= result + bit) {
//				num -= result + bit;
//				result = (result >> 1) + bit;
//			}
//			else
//				result >>= 1;
//
//			bit >>= 2;
//		}
//
//		return result;
// }
//
struct word_header* word_op_isqrt(struct word_header* radicand) {
	
	assert(NULL != radicand && radicand->length > 0);
	if (NULL == radicand || 0 == radicand->length) {
		perror("Invalid word was passed to word_op_isqrt.");
		return NULL;
	}
	
	struct word_header *bit = create_word(radicand->length),
	*result = create_word(radicand->length),
	*remainder = create_word_duplicate(radicand);
	
	BITS(bit)[~((int)1) & (radicand->length - 1)] = 1;
	
	// while (bit > remainder)
	while (1 == word_op_compare(bit, remainder))
		word_op_rightshift(bit, 2);
	
	// while (bit != 0)
	while (0 != word_op_compare_constant(bit, 0)) {
		
		// update_term = (result + bit);
		struct word_header* update_term = create_word_duplicate(result);
		word_op_add(update_term, bit, 1, 0);
		
		// if (remainder >= (result + bit) OR >= (update_term)) {
		if (word_op_compare(remainder, update_term) >= 0) {
			// remainder -= (result + bit) OR (update_term);
			word_op_add(remainder, update_term, -1, 0);
			
			// result = (result >> 1) + bit;
			word_op_rightshift(result, 1);
			word_op_add(result, bit, 1, 0);
		}
		else {
			// result >>= 1;
			word_op_rightshift(result, 1);
		}
		
		// bit >>= 2;
		word_op_rightshift(bit, 2);
		word_deallocate(update_term);
	}
	
	word_deallocate(bit);
	word_deallocate(remainder);
	
	return result;
}


