#include "../include/prediction_module.h"

/*
 * ПРГ:Тс-5 - Общая логика обновления двумерного массива P'
 * Формула: P' = P
 */

bool Matrix_Copy_Atomic(const double* src, double* dst, uint32_t rows, uint32_t cols)
{
    if (!src || !dst || rows == 0 || cols == 0) return false;
    if (src == dst) return true;
    
    uint32_t elements = rows * cols;
    if (elements < rows || elements < cols) return false;
    
    for (uint32_t i = 0; i < elements; i++) dst[i] = src[i];
    return true;
}

bool Prediction_CopyPpredToPest(PredictionConfig* config)
{
    if (!config || !config->p_pred_computed) return false;
    if (!config->P_pred || !config->P_est) return false;
    
    return Matrix_Copy_Atomic(config->P_pred, config->P_est, config->n, config->n);
}

bool Prediction_Step4_UpdatePEst(PredictionConfig* config)
{
    if (!config || !config->p_pred_computed) return false;
    return Prediction_CopyPpredToPest(config);
}
