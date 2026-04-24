#include "../include/prediction_module.h"

/*
 * ПРГ:Тс-4 - Общая логика обновления массива X'
 * Формула: X' = X
 */

bool Vector_Copy_Atomic(const double* src, double* dst, uint32_t size)
{
    if (!src || !dst || size == 0) return false;
    if (src == dst) return true;
    
    for (uint32_t i = 0; i < size; i++) dst[i] = src[i];
    return true;
}

bool Prediction_CopyXpredToXest(PredictionConfig* config)
{
    if (!config || !config->x_pred_computed) return false;
    if (!config->X_pred || !config->X_est) return false;
    
    return Vector_Copy_Atomic(config->X_pred, config->X_est, config->n);
}

bool Prediction_Step2_UpdateXEst(PredictionConfig* config)
{
    if (!config || !config->x_pred_computed) return false;
    return Prediction_CopyXpredToXest(config);
}
