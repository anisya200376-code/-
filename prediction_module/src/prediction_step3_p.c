#include "../include/prediction_module.h"
#include <stdlib.h>
#include <math.h>
#include <float.h>

/*
 * ПРГ:Тс-3 - Общая логика обновления прогнозируемой матрицы ковариации P
 * Формула: P = F × P' × F^T + G × Q × G^T
 */

bool Prediction_ComputePPred(PredictionConfig* config, double* result)
{
    if (!config || !result) return false;
    
    uint32_t n = config->n, p = config->p;
    
    if (!config->inter_allocated && !Prediction_AllocateIntermediate(config))
        return false;
    
    bool overflow = false, underflow = false;
    
    // F × P'
    Matrix_Multiply_WithCheck(config->F, config->P_est, config->inter.temp_FP,
                               n, n, n, &overflow, &underflow);
    if (overflow) { config->overflow_detected = true; return false; }
    
    // (F × P') × F^T
    Matrix_TransposeMultiply(config->inter.temp_FP, config->F, config->inter.temp_FPFt,
                              n, n, n);
    
    // G × Q
    Matrix_Multiply_WithCheck(config->G, config->Q, config->inter.temp_GQ,
                               n, p, p, &overflow, &underflow);
    if (overflow) { config->overflow_detected = true; return false; }
    
    // (G × Q) × G^T
    Matrix_TransposeMultiply(config->inter.temp_GQ, config->G, config->inter.temp_GQGt,
                              n, p, n);
    
    // Сложение
    Matrix_Copy(config->inter.temp_FPFt, result, n * n);
    Matrix_Add_WithCheck(result, config->inter.temp_GQGt, result,
                         n, n, &overflow, &underflow);
    
    if (overflow) { config->overflow_detected = true; return false; }
    if (underflow) config->underflow_detected = true;
    
    return true;
}
