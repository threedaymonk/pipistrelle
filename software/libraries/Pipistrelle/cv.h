#ifndef __cv_h__
#define __cv_h__

#define END_SLOP 5

float unipolar(int reading);
float bipolar(int reading);
float unipolar_with_cv(int pot, int cv);

#endif /* __cv_h__ */
