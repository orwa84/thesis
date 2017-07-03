/*
 *  randomizer.h
 *  mechanical project
 *
 *  Created by Orwa on 1/6/12.
 *
 */

char initialize_randomizer() {
	
	int return_value = 0;
	struct timeval current_time;
	
	return_value = gettimeofday(&current_time, NULL);
	
	assert(0 == return_value);
	if (-1 == return_value) {
		perror("Couldn't retrieve time using \"gettimeofday\".");
		return -1;
	}
	
	// "tv_sec" is the number of seconds since the epoch.
	// "tv_usec" is a fractional second value, corresponding to
	//   the number of microseconds (one over a million of a sec).
	srand(current_time.tv_sec / 1000 + current_time.tv_usec * 1000);
	
	return 0;
}

unsigned char random_bit() {
	return (unsigned char) (rand() > (RAND_MAX >> 1) ? 1 : 0);
}

