/********************************************************************************************************************** 
 * THE LOOKING GLASS VISUALIZATION TOOLSET
 *---------------------------------------------------------------------------------------------------------------------
 * Author: 
 *	Alessandro Febretti							Electronic Visualization Laboratory, University of Illinois at Chicago
 * Contact & Web:
 *  febret@gmail.com							http://febretpository.hopto.org
 *---------------------------------------------------------------------------------------------------------------------
 * Looking Glass has been built as part of the ENDURANCE Project (http://www.evl.uic.edu/endurance/).
 * ENDURANCE is supported by the NASA ASTEP program under Grant NNX07AM88G and by the NSF USAP.
 *********************************************************************************************************************/ 
#ifndef EVALFUNCTIONS_H
#define EVALFUNCTIONS_H

double sndVelC(double s, double t, double p0)
{
	// s = salinity, t = temperature deg C ITPS-68, p = pressure in decibars
	double a, a0, a1, a2, a3;
	double b, b0, b1;
	double c, c0, c1, c2, c3;
	double p, sr, d, sv;
	p = p0 / 10.0; /* scale pressure to bars */
	if (s < 0.0) s = 0.0;
	sr = sqrt(s);
	d = 1.727e-3 - 7.9836e-6 * p;
	b1 = 7.3637e-5 + 1.7945e-7 * t;
	b0 = -1.922e-2 - 4.42e-5 * t;
	b = b0 + b1 * p;
	a3 = (-3.389e-13 * t + 6.649e-12) * t + 1.100e-10;
	a2 = ((7.988e-12 * t - 1.6002e-10) * t + 9.1041e-9) * t - 3.9064e-7;
	a1 = (((-2.0122e-10 * t + 1.0507e-8) * t - 6.4885e-8) * t - 1.2580e-5)	* t + 9.4742e-5;
	a0 = (((-3.21e-8 * t + 2.006e-6) * t + 7.164e-5) * t -1.262e-2) * t + 1.389;
	a = ((a3 * p + a2) * p + a1) * p + a0;
	c3 = (-2.3643e-12 * t + 3.8504e-10) * t - 9.7729e-9;
	c2 = (((1.0405e-12 * t -2.5335e-10) * t + 2.5974e-8) * t - 1.7107e-6)	* t + 3.1260e-5;
	c1 = (((-6.1185e-10 * t + 1.3621e-7) * t - 8.1788e-6) * t + 6.8982e-4)	* t + 0.153563;
	c0 = ((((3.1464e-9 * t - 1.47800e-6) * t + 3.3420e-4) * t - 	5.80852e-2) * t + 5.03711) * t +	1402.388;
	c = ((c3 * p + c2) * p + c1) * p + c0;
	sv = c + (a + b * sr + d * s) * s;
	return sv;
}

#endif
