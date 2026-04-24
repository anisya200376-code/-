#include "../include/prediction_module.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 * ПРГ:Ту-1 - Общий состав входных данных
 * ПРГ:Тр-1 - Общий результат выполнения модуля
 */

bool Prediction_Init(PredictionConfig* config,
                      uint32_t n,
                      uint32_t p,
                      double* X_est,
                      double* P_est,
                      double* X_pred,
                      double* P_pred,
                      const double* F,
                      const double* G,
                      const double* Q)
{
    if (config == NULL) return false;
    
    config->n = n;
    config->p = p;
    config->X_est = X_est;
    config->P_est = P_est;
    config->F = F;
    config->G = G;
    config->Q = Q;
    config->X_pred = X_pred;
    config->P_pred = P_pred;
    
    config->status.x_pred_updated = false;
    config->status.p_pred_updated = false;
    config->overflow_detected = false;
    config->underflow_detected = false;
    config->x_pred_computed = false;
    config->p_pred_computed = false;
    
    config->x_min_bound = -1.0e30;
    config->x_max_bound = 1.0e30;
    
    config->x_diagnostic.status = X_VALID_OK;
    config->x_diagnostic.error_index = 0;
    config->x_diagnostic.error_value = 0.0;
    config->x_diagnostic.min_bound = config->x_min_bound;
    config->x_diagnostic.max_bound = config->x_max_bound;
    
    config->p_diagnostic.status = P_VALID_OK;
    config->p_diagnostic.error_row = 0;
    config->p_diagnostic.error_col = 0;
    config->p_diagnostic.error_value = 0.0;
    
    config->inter.temp_FPFt = NULL;
    config->inter.temp_GQGt = NULL;
    config->inter.temp_FP = NULL;
    config->inter.temp_GQ = NULL;
    config->inter_allocated = false;
    
    return (n > 0 && p > 0 && X_est && P_est && X_pred && P_pred && F && G && Q);
}

bool Prediction_AllocateIntermediate(PredictionConfig* config)
{
    if (!config || config->n == 0 || config->p == 0) return false;
    
    uint32_t n = config->n, p = config->p;
    
    config->inter.temp_FPFt = (double*)malloc(n * n * sizeof(double));
    config->inter.temp_GQGt = (double*)malloc(n * n * sizeof(double));
    config->inter.temp_FP = (double*)malloc(n * n * sizeof(double));
    config->inter.temp_GQ = (double*)malloc(n * p * sizeof(double));
    
    if (!config->inter.temp_FPFt || !config->inter.temp_GQGt ||
        !config->inter.temp_FP || !config->inter.temp_GQ) {
        Prediction_FreeIntermediate(config);
        return false;
    }
    
    config->inter_allocated = true;
    return true;
}

void Prediction_FreeIntermediate(PredictionConfig* config)
{
    if (!config) return;
    free(config->inter.temp_FPFt);
    free(config->inter.temp_GQGt);
    free(config->inter.temp_FP);
    free(config->inter.temp_GQ);
    config->inter.temp_FPFt = config->inter.temp_GQGt = NULL;
    config->inter.temp_FP = config->inter.temp_GQ = NULL;
    config->inter_allocated = false;
}

void Prediction_SetXBounds(PredictionConfig* config, double min_bound, double max_bound)
{
    if (config) {
        config->x_min_bound = min_bound;
        config->x_max_bound = max_bound;
        config->x_diagnostic.min_bound = min_bound;
        config->x_diagnostic.max_bound = max_bound;
    }
}
