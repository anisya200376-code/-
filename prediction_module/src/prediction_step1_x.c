#include "../include/prediction_module.h"
#include <math.h>
#include <float.h>

/*
 * ПРГ:Тс-2 - Общая логика обновления прогнозируемого вектора состояния
 * Формула: X = F × X'
 */

bool Prediction_ComputeXPred(PredictionConfig* config, double* result)
{
    if (!config || !result || !config->F || !config->X_est) return false;
    
    uint32_t n = config->n;
    
    for (uint32_t i = 0; i < n; i++) {
        long double sum = 0.0L;
        for (uint32_t j = 0; j < n; j++) {
            sum += (long double)config->F[i * n + j] * (long double)config->X_est[j];
        }
        result[i] = (double)sum;
        if (isinf(result[i])) {
            config->overflow_detected = true;
            return false;
        }
        if (fabs(result[i]) < DBL_MIN && result[i] != 0.0) {
            config->underflow_detected = true;
        }
    }
    return true;
}

bool Prediction_ValidateMatrixFForX(const PredictionConfig* config, XVectorDiagnostic* diag)
{
    if (!config || !config->F) {
        if (diag) diag->status = X_VALID_F_NULL;
        return false;
    }
    
    uint32_t n = config->n, row, col;
    double val;
    
    if (Matrix_HasNaN(config->F, n, n, &row, &col, &val)) {
        if (diag) { diag->status = X_VALID_F_HAS_NAN; diag->error_index = row * n + col; diag->error_value = val; }
        return false;
    }
    
    if (Matrix_HasInf(config->F, n, n, &row, &col, &val)) {
        if (diag) { diag->status = X_VALID_F_HAS_INF; diag->error_index = row * n + col; diag->error_value = val; }
        return false;
    }
    
    if (diag) diag->status = X_VALID_OK;
    return true;
}

bool Prediction_ValidateVectorXEst(const PredictionConfig* config, XVectorDiagnostic* diag)
{
    if (!config || !config->X_est) {
        if (diag) diag->status = X_VALID_X_EST_NULL;
        return false;
    }
    
    uint32_t n = config->n, idx;
    double val;
    
    if (Vector_HasNaN(config->X_est, n, &idx, &val)) {
        if (diag) { diag->status = X_VALID_X_EST_HAS_NAN; diag->error_index = idx; diag->error_value = val; }
        return false;
    }
    
    if (Vector_HasInf(config->X_est, n, &idx, &val)) {
        if (diag) { diag->status = X_VALID_X_EST_HAS_INF; diag->error_index = idx; diag->error_value = val; }
        return false;
    }
    
    if (diag) diag->status = X_VALID_OK;
    return true;
}

bool Prediction_ValidateResultX(const PredictionConfig* config, XVectorDiagnostic* diag)
{
    if (!config || !config->X_pred) return false;
    
    uint32_t n = config->n, idx;
    double val;
    
    if (Vector_HasNaN(config->X_pred, n, &idx, &val)) {
        if (diag) { diag->status = X_VALID_RESULT_HAS_NAN; diag->error_index = idx; diag->error_value = val; }
        return false;
    }
    
    if (Vector_HasInf(config->X_pred, n, &idx, &val)) {
        if (diag) { diag->status = X_VALID_RESULT_HAS_INF; diag->error_index = idx; diag->error_value = val; }
        return false;
    }
    
    if (!Vector_WithinBounds(config->X_pred, n, config->x_min_bound, config->x_max_bound, &idx, &val)) {
        if (diag) { diag->status = X_VALID_RESULT_OUT_OF_BOUNDS; diag->error_index = idx; diag->error_value = val; }
        return false;
    }
    
    if (diag) diag->status = X_VALID_OK;
    return true;
}

bool Prediction_Step1_UpdateXPred(PredictionConfig* config)
{
    if (!config) { if (config) config->x_pred_computed = false; return false; }
    
    if (!Prediction_ValidateMatrixFForX(config, &config->x_diagnostic) ||
        !Prediction_ValidateVectorXEst(config, &config->x_diagnostic)) {
        config->x_pred_computed = false;
        return false;
    }
    
    if (!Prediction_ComputeXPred(config, config->X_pred)) {
        config->x_diagnostic.status = X_VALID_CALC_OVERFLOW;
        config->x_pred_computed = false;
        return false;
    }
    
    if (!Prediction_ValidateResultX(config, &config->x_diagnostic)) {
        config->x_pred_computed = false;
        return false;
    }
    
    config->x_pred_computed = true;
    return true;
}
