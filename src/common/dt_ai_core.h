/*
    This file is part of darktable,
    Copyright (C) 2026 darktable developers.

    darktable is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    darktable is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with darktable.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <glib.h>
#include <stdint.h>
#include "common/darktable.h"

/**
 * \brief AI Core: Abstraction layer for Deep Learning inference.
 *
 * This module provides a unified interface to load and execute machine learning
 * models (currently supporting ONNX Runtime). It handles the complexity
 * of session management, input/output tensor binding, and execution device selection
 * (CPU, CUDA, TensorRT, CoreML, etc., if available).
 */

typedef struct dt_ai_model_t dt_ai_model_t;

/**
 * \brief Tensor data types supported by the AI Core.
 */
typedef enum dt_ai_tensor_type_t {
  DT_AI_TENSOR_FLOAT = 1,
  DT_AI_TENSOR_UINT8 = 2,
  DT_AI_TENSOR_INT64 = 3,
} dt_ai_tensor_type_t;

/**
 * \brief Describes a tensor's shape and data.
 */
typedef struct dt_ai_tensor_t {
  void *data;                 // Pointer to raw data buffer
  size_t data_size;           // Size of buffer in bytes
  dt_ai_tensor_type_t type;   // Data type
  int64_t *shape;             // Array of dimensions (e.g., {1, 3, 256, 256})
  size_t shape_len;           // Number of dimensions
  char *name;                 // Name of the input/output node in the model
} dt_ai_tensor_t;

/**
 * \brief Initialize the AI Core runtime.
 * Should be called once at startup.
 */
void dt_ai_init();

/**
 * \brief Cleanup the AI Core runtime.
 * Should be called at shutdown.
 */
void dt_ai_cleanup();

/**
 * \brief Load an AI model from a file path.
 *
 * \param model_path Absolute path to the .onnx model file.
 * \param use_gpu If TRUE, attempts to use GPU execution provider (CUDA/CoreML).
 *                Falls back to CPU if unavailable.
 * \return Pointer to the loaded model, or NULL on failure.
 */
dt_ai_model_t *dt_ai_load_model(const char *model_path, gboolean use_gpu);

/**
 * \brief Run inference on the loaded model.
 *
 * \param model The loaded model instance.
 * \param inputs Array of input tensors.
 * \param num_inputs Number of input tensors.
 * \param outputs Array of output tensors (pre-allocated data buffers).
 * \param num_outputs Number of output tensors.
 * \return TRUE on success, FALSE on failure.
 */
gboolean dt_ai_run_inference(dt_ai_model_t *model,
                             dt_ai_tensor_t *inputs, size_t num_inputs,
                             dt_ai_tensor_t *outputs, size_t num_outputs);

/**
 * \brief Unload and free the model resources.
 */
void dt_ai_unload_model(dt_ai_model_t *model);

/**
 * \brief Check if AI Core is available (compiled with support and runtime working).
 */
gboolean dt_ai_is_available();
