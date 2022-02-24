#ifndef PIPISTRELLE_CV_H_
#define PIPISTRELLE_CV_H_

#define END_SLOP 5

float unipolar(int reading);
float bipolar(int reading);
float unipolar_with_cv(int pot, int cv);

#endif  // PIPISTRELLE_CV_H_
