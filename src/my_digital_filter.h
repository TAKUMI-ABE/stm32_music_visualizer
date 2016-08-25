#ifndef FIR_FILTER_H
#define FIR_FILTER_H

#include <cmath>

/*
 * @brief main FFT function
 */
template<class T>
void IIR_LPF(T fc, T Q, T a[], T b[]){
	fc = tan(M_PI * fc) / (2.0 * M_PI);

	a[0] = 1.0 + 2.0 * M_PI * fc / Q + 4.0 * M_PI * M_PI * fc * fc;
	a[1] = (8.0 * M_PI * M_PI * fc * fc - 2.0) / a[0];
	a[2] = (1.0 - 2.0 * M_PI * fc / Q + 4.0 * M_PI * M_PI * fc * fc) / a[0];
	b[0] = 4.0 * M_PI * M_PI * fc * fc / a[0];
	b[1] = 8.0 * M_PI * M_PI * fc * fc / a[0];
	b[2] = 4.0 * M_PI * M_PI * fc * fc / a[0];

	a[0] = 1.0;
}

#endif // FIR_FILTER_H
