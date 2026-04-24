#ifndef PREDICTION_MODULE_H
#define PREDICTION_MODULE_H

/*
 * Модуль прогнозирования для подзадачи КОИ «СКОР»
 * Соответствует требованиям КТ-178 (авиационные стандарты)
 *
 * Требования, реализованные в модуле:
 * - ПРГ:Ту-1    Общий состав входных данных
 * - ПРГ:Тр-1    Общий результат выполнения модуля
 * - ПРГ:Тс-1    Назначение и общая логика выполнения
 * - ПРГ:Тс-2    Общая логика обновления вектора X (X = F × X')
 * - ПРГ:Тс-3    Общая логика обновления матрицы P (P = F×P'×F^T + G×Q×G^T)
 * - ПРГ:Тс-4    Общая логика обновления X' (X' = X)
 * - ПРГ:Тс-5    Общая логика обновления P' (P' = P)
 * - ПРГ:Тс-6    Логика формирования признака для P
 * - ПРГ:Тс-7    Логика формирования признака для X
 */

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * Типы данных для диагностики (ПРГ:Тс-6, ПРГ:Тс-7)
 * ============================================================================ */

// Причины недостоверности для вектора X (ПРГ:Тс-7)
typedef enum {
    X_VALID_OK = 0,
    X_VALID_F_NULL = 1,
    X_VALID_X_EST_NULL = 2,
    X_VALID_F_HAS_NAN = 3,
    X_VALID_F_HAS_INF = 4,
    X_VALID_X_EST_HAS_NAN = 5,
    X_VALID_X_EST_HAS_INF = 6,
    X_VALID_CALC_OVERFLOW = 7,
    X_VALID_RESULT_HAS_NAN = 8,
    X_VALID_RESULT_HAS_INF = 9,
    X_VALID_RESULT_OUT_OF_BOUNDS = 10
} XValidationStatus;

// Причины недостоверности для матрицы P (ПРГ:Тс-6)
typedef enum {
    P_VALID_OK = 0,
    P_VALID_F_NULL = 1,
    P_VALID_G_NULL = 2,
    P_VALID_Q_NULL = 3,
    P_VALID_F_HAS_NAN = 4,
    P_VALID_F_HAS_INF = 5,
    P_VALID_G_HAS_NAN = 6,
    P_VALID_G_HAS_INF = 7,
    P_VALID_Q_HAS_NAN = 8,
    P_VALID_Q_HAS_INF = 9,
    P_VALID_Q_NOT_SYMMETRIC = 10,
    P_VALID_Q_NOT_POSITIVE_DEF = 11,
    P_VALID_CALC_OVERFLOW = 12,
    P_VALID_CALC_NAN = 13,
    P_VALID_CALC_INF = 14,
    P_VALID_PRED_NOT_SYMMETRIC = 15,
    P_VALID_PRED_NOT_POSITIVE_DEF = 16
} PValidationStatus;

// Диагностика для вектора X (ПРГ:Тс-7)
typedef struct {
    XValidationStatus status;
    uint32_t error_index;
    double error_value;
    double min_bound;
    double max_bound;
} XVectorDiagnostic;

// Диагностика для матрицы P (ПРГ:Тс-6)
typedef struct {
    PValidationStatus status;
    uint32_t error_row;
    uint32_t error_col;
    double error_value;
} PValidationDiagnostic;

/* ============================================================================
 * Структуры состояния (ПРГ:Тр-1, ПРГ:Ту-1)
 * ============================================================================ */

// Статус выполнения (ПРГ:Тр-1)
typedef struct {
    bool x_pred_updated;    // Признак "Обновлен прогноз вектора X" (ПРГ:Тс-7)
    bool p_pred_updated;    // Признак "Обновлен прогноз матрицы P" (ПРГ:Тс-6)
} PredictionStatus;

// Промежуточные результаты (ПРГ:Тс-3)
typedef struct {
    double* temp_FPFt;      // F × P' × F^T
    double* temp_GQGt;      // G × Q × G^T
    double* temp_FP;        // F × P'
    double* temp_GQ;        // G × Q
} PredictionIntermediate;

// Основная конфигурация модуля (ПРГ:Ту-1)
typedef struct {
    // Размерности
    uint32_t n;                     // Размерность вектора состояния
    uint32_t p;                     // Размерность вектора входных шумов
    
    // Входные параметры
    double* X_est;                  // Оценка X'
    double* P_est;                  // Оценка P'
    const double* F;                // Переходная матрица (n×n)
    const double* G;                // Матрица влияния (n×p)
    const double* Q;                // Ковариационная матрица (p×p)
    
    // Выходные параметры
    double* X_pred;                 // Прогнозируемый X
    double* P_pred;                 // Прогнозируемая P
    
    // Статус и диагностика
    PredictionStatus status;
    XVectorDiagnostic x_diagnostic; // Диагностика для X (ПРГ:Тс-7)
    PValidationDiagnostic p_diagnostic; // Диагностика для P (ПРГ:Тс-6)
    
    // Флаги арифметических исключений
    bool overflow_detected;
    bool underflow_detected;
    
    // Флаги последовательности выполнения
    bool x_pred_computed;
    bool p_pred_computed;
    
    // Границы для X (ПРГ:Тс-7)
    double x_min_bound;
    double x_max_bound;
    
    // Промежуточные данные (ПРГ:Тс-3)
    PredictionIntermediate inter;
    bool inter_allocated;
} PredictionConfig;

/* ============================================================================
 * Основные функции модуля (ПРГ:Тс-1, ПРГ:Ту-1)
 * ============================================================================ */

// Инициализация модуля (ПРГ:Ту-1)
bool Prediction_Init(PredictionConfig* config,
                      uint32_t n,
                      uint32_t p,
                      double* X_est,
                      double* P_est,
                      double* X_pred,
                      double* P_pred,
                      const double* F,
                      const double* G,
                      const double* Q);

// Основная функция КОИ «СКОР» (ПРГ:Тс-1)
bool Prediction_Run_KOI(PredictionConfig* config);

// Установка границ для X (ПРГ:Тс-7)
void Prediction_SetXBounds(PredictionConfig* config, double min_bound, double max_bound);

// Управление промежуточной памятью
bool Prediction_AllocateIntermediate(PredictionConfig* config);
void Prediction_FreeIntermediate(PredictionConfig* config);

/* ============================================================================
 * Функции шагов (ПРГ:Тс-2, ПРГ:Тс-3, ПРГ:Тс-4, ПРГ:Тс-5)
 * ============================================================================ */

// Шаг 1: X = F × X' (ПРГ:Тс-2)
bool Prediction_Step1_UpdateXPred(PredictionConfig* config);

// Шаг 2: X' = X (ПРГ:Тс-4)
bool Prediction_Step2_UpdateXEst(PredictionConfig* config);

// Шаг 3: P = F×P'×F^T + G×Q×G^T (ПРГ:Тс-3)
bool Prediction_Step3_UpdatePPred(PredictionConfig* config);

// Шаг 4: P' = P (ПРГ:Тс-5)
bool Prediction_Step4_UpdatePEst(PredictionConfig* config);

/* ============================================================================
 * Функции проверки достоверности (ПРГ:Тс-6, ПРГ:Тс-7)
 * ============================================================================ */

// Проверки для X (ПРГ:Тс-7)
bool Prediction_ValidateMatrixFForX(const PredictionConfig* config, XVectorDiagnostic* diag);
bool Prediction_ValidateVectorXEst(const PredictionConfig* config, XVectorDiagnostic* diag);
bool Prediction_ValidateResultX(const PredictionConfig* config, XVectorDiagnostic* diag);

// Проверки для P (ПРГ:Тс-6)
bool Prediction_ValidateMatrixF(const PredictionConfig* config, PValidationDiagnostic* diag);
bool Prediction_ValidateMatrixG(const PredictionConfig* config, PValidationDiagnostic* diag);
bool Prediction_ValidateMatrixQ(const PredictionConfig* config, PValidationDiagnostic* diag);
bool Prediction_ValidateResultP(const PredictionConfig* config, PValidationDiagnostic* diag);

/* ============================================================================
 * Вспомогательные математические функции
 * ============================================================================ */

// Основные операции
void Matrix_Multiply(const double* A, const double* B, double* result,
                     uint32_t rows_A, uint32_t cols_A, uint32_t cols_B);
void Matrix_TransposeMultiply(const double* A, const double* B, double* result,
                               uint32_t rows_A, uint32_t cols_A, uint32_t rows_B);
void Matrix_Add(const double* A, const double* B, double* result,
                uint32_t rows, uint32_t cols);
void Matrix_Copy(const double* src, double* dst, uint32_t elements);

// Операции с проверками
void Matrix_Multiply_WithCheck(const double* A, const double* B, double* result,
                                uint32_t rows_A, uint32_t cols_A, uint32_t cols_B,
                                bool* overflow, bool* underflow);
void Matrix_Add_WithCheck(const double* A, const double* B, double* result,
                          uint32_t rows, uint32_t cols,
                          bool* overflow, bool* underflow);

// Проверки свойств
bool Matrix_HasNaN(const double* mat, uint32_t rows, uint32_t cols,
                   uint32_t* row, uint32_t* col, double* value);
bool Matrix_HasInf(const double* mat, uint32_t rows, uint32_t cols,
                   uint32_t* row, uint32_t* col, double* value);
bool Matrix_IsSymmetric(const double* mat, uint32_t n,
                        uint32_t* row, uint32_t* col, double* diff);
bool Matrix_IsPositiveDefinite(const double* mat, uint32_t n);

bool Vector_HasNaN(const double* vec, uint32_t size, uint32_t* index, double* value);
bool Vector_HasInf(const double* vec, uint32_t size, uint32_t* index, double* value);
bool Vector_WithinBounds(const double* vec, uint32_t size, double min_bound, double max_bound,
                         uint32_t* index, double* value);

// Атомарное копирование (КТ-178)
bool Vector_Copy_Atomic(const double* src, double* dst, uint32_t size);
bool Matrix_Copy_Atomic(const double* src, double* dst, uint32_t rows, uint32_t cols);

#endif // PREDICTION_MODULE_H
