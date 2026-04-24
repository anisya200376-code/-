#include "../include/prediction_module.h"
#include <math.h>
#include <float.h>

/*
 * ПРГ:Тс-6 - Общая логика обновления признака обновления прогноза матрицы P
 * Признак = 1 если F, G, Q достоверны и расчёт P выполнен правильно
 * Признак = 0 в противном случае
 */

bool Prediction_ValidateMatrixF(const PredictionConfig* config, PValidationDiagnostic* diag)
{
    if (!config || !config->F) {
        if (diag) diag->status = P_VALID_F_NULL;
        return false;
    }
    
    uint32_t n = config->n, row, col;
    double val;
    
    if (Matrix_HasNaN(config->F, n, n, &row, &col, &val)) {
        if (diag) { diag->status = P_VALID_F_HAS_NAN; diag->error_row = row; diag->error_col = col; diag->error_value = val; }
        return false;
    }
    
    if (Matrix_HasInf(config->F, n, n, &row, &col, &val)) {
        if (diag) { diag->status = P_VALID_F_HAS_INF; diag->error_row = row; diag->error_col = col; diag->error_value = val; }
        return false;
    }
    
    if (diag) diag->status = P_VALID_OK;
    return true;
}

bool Prediction_ValidateMatrixG(const PredictionConfig* config, PValidationDiagnostic* diag)
{
    if (!config || !config->G) {
        if (diag) diag->status = P_VALID_G_NULL;
        return false;
    }
    
    uint32_t n = config->n, p = config->p, row, col;
    double val;
    
    if (Matrix_HasNaN(config->G, n, p, &row, &col, &val)) {
        if (diag) { diag->status = P_VALID_G_HAS_NAN; diag->error_row = row; diag->error_col = col; diag->error_value = val; }
        return false;
    }
    
    if (Matrix_HasInf(config->G, n, p, &row, &col, &val)) {
        if (diag) { diag->status = P_VALID_G_HAS_INF; diag->error_row = row; diag->error_col = col; diag->error_value = val; }
        return false;
    }
    
    if (diag) diag->status = P_VALID_OK;
    return true;
}

bool Prediction_ValidateMatrixQ(const PredictionConfig* config, PValidationDiagnostic* diag)
{
    if (!config || !config->Q) {
        if (diag) diag->status = P_VALID_Q_NULL;
        return false;
    }
    
    uint32_t p = config->p, row, col;
    double val, diff;
    
    if (Matrix_HasNaN(config->Q, p, p, &row, &col, &val)) {
        if (diag) { diag->status = P_VALID_Q_HAS_NAN; diag->error_row = row; diag->error_col = col; diag->error_value = val; }
        return false;
    }
    
    if (Matrix_HasInf(config->Q, p, p, &row, &col, &val)) {
        if (diag) { diag->status = P_VALID_Q_HAS_INF; diag->error_row = row; diag->error_col = col; diag->error_value = val; }
        return false;
    }
    
    if (!Matrix_IsSymmetric(config->Q, p, &row, &col, &diff)) {
        if (diag) { diag->status = P_VALID_Q_NOT_SYMMETRIC; diag->error_row = row; diag->error_col = col; diag->error_value = diff; }
        return false;
    }
    
    if (!Matrix_IsPositiveDefinite(config->Q, p)) {
        if (diag) diag->status = P_VALID_Q_NOT_POSITIVE_DEF;
        return false;
    }
    
    if (diag) diag->status = P_VALID_OK;
    return true;
}

bool Prediction_ValidateResultP(const PredictionConfig* config, PValidationDiagnostic* diag)
{
    if (!config || !config->P_pred) return false;
    
    uint32_t n = config->n, row, col;
    double val, diff;
    
    if (Matrix_HasNaN(config->P_pred, n, n, &row, &col, &val)) {
        if (diag) { diag->status = P_VALID_CALC_NAN; diag->error_row = row; diag->error_col = col; diag->error_value = val; }
        return false;
    }
    
    if (Matrix_HasInf(config->P_pred, n, n, &row, &col, &val)) {
        if (diag) { diag->status = P_VALID_CALC_INF; diag->error_row = row; diag->error_col = col; diag->error_value = val; }
        return false;
    }
    
    if (!Matrix_IsSymmetric(config->P_pred, n, &row, &col, &diff)) {
        if (diag) { diag->status = P_VALID_PRED_NOT_SYMMETRIC; diag->error_row = row; diag->error_col = col; diag->error_value = diff; }
        return false;
    }
    
    if (!Matrix_IsPositiveDefinite(config->P_pred, n)) {
        if (diag) diag->status = P_VALID_PRED_NOT_POSITIVE_DEF;
        return false;
    }
    
    if (diag) diag->status = P_VALID_OK;
    return true;
}

bool Prediction_Step3_UpdatePPred(PredictionConfig* config)
{
    if (!config) return false;
    
    if (!Prediction_ValidateMatrixF(config, &config->p_diagnostic) ||
        !Prediction_ValidateMatrixG(config, &config->p_diagnostic) ||
        !Prediction_ValidateMatrixQ(config, &config->p_diagnostic)) {
        config->p_pred_computed = false;
        return false;
    }
    
    if (!Prediction_ComputePPred(config, config->P_pred)) {
        config->p_diagnostic.status = P_VALID_CALC_OVERFLOW;
        config->p_pred_computed = false;
        return false;
    }
    
    if (!Prediction_ValidateResultP(config, &config->p_diagnostic)) {
        config->p_pred_computed = false;
        return false;
    }
    
    config->p_pred
