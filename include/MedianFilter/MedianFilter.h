 #ifndef MEDIANFILTER_H
 #define MEDIANFILTER_H

 #ifdef __cplusplus
extern "C" {
#endif

struct node
   {
      struct node *next;
      float value;
   };

float MedianFilter(float newValue);
float Median3(float a,float b,float c);
float AddValue3(float value);
void ArrayDisplacement(float *array,int dim);

#ifdef __cplusplus
}
#endif

 #endif
