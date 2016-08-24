#ifndef MY_FFT_H
#define MY_FFT_H

#include <cmath>
#include <vector>

/*
 * @brief y = log2(x)
 */
inline int log2(int x){
	int y;
	y = 0;
	while (x > 1){
		x >>= 1;
		y++;
	}

	return y;
}

/*
 * @brief y = 2 ^ x
 */
inline int pow2(int x){
	int y;

	(x == 0)? y = 1 : y = 2 << (x-1);

	return y;
}

/*
 * @brief main FFT function
 */
template<class T>
void tFFT(std::vector<T>& x_real, std::vector<T>& x_imag, int N){
	int n, m, r, number_of_stage;
	std::vector<int> index(N);
	T a_real, a_imag, b_real, b_imag, c_real, c_imag, real, imag;

	// FFT stages
	number_of_stage = log2(N);

	// Butterfly Computation
	for (int stage = 1; stage <= number_of_stage; stage++){
		for (int i = 0; i < pow2(stage - 1); i++){
			for (int j = 0; j < pow2(number_of_stage - stage); j++){
				n = pow2(number_of_stage - stage + 1) * i + j;
				m = pow2(number_of_stage - stage) + n;
				r = pow2(stage - 1) * j;
				a_real = x_real[n];
				a_imag = x_imag[n];
				b_real = x_real[m];
				b_imag = x_imag[m];
				c_real = cos((2.0 * M_PI * r) / N);
				c_imag = -sin((2.0 * M_PI * r) / N);

				if (stage < number_of_stage){
					x_real[n] = a_real + b_real;
					x_imag[n] = a_imag + b_imag;
					x_real[m] = (a_real - b_real) * c_real - (a_imag - b_imag) * c_imag;
					x_imag[m] = (a_imag - b_imag) * c_real + (a_real - b_real) * c_imag;
				}
				else{
					x_real[n] = a_real + b_real;
					x_imag[n] = a_imag + b_imag;
					x_real[m] = a_real - b_real;
					x_imag[m] = a_imag - b_imag;
				}
			}
		}
	}

	// Making table for indexing
	for (int stage = 1; stage <= number_of_stage; stage++){
		for (int i = 0; i < pow2(stage - 1); i++){
			index[pow2(stage - 1) + i] = index[i] + pow2(number_of_stage - stage);
		}
	}

	// Sort of index
	for (int k = 0; k < N; k++){
		if (index[k] > k){
			real = x_real[index[k]];
			imag = x_imag[index[k]];
			x_real[index[k]] = x_real[k];
			x_imag[index[k]] = x_imag[k];
			x_real[k] = real;
			x_imag[k] = imag;
		}
	}
}

#endif // FFT_H
