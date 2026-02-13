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

#include "common/dt_ai_core.h"
#include "common/debug.h"

#ifdef HAVE_ONNXRUNTIME
#include <onnxruntime_c_api.h>
#endif

/*
 * Internal structure for the model
 */
struct dt_ai_model_t {
#ifdef HAVE_ONNXRUNTIME
  OrtSession *session;
  OrtEnv *env;
  OrtSessionOptions *session_options;
  OrtMemoryInfo *memory_info;
#else
  void *dummy;
#endif
};

#ifdef HAVE_ONNXRUNTIME
static const OrtApi *g_ort = NULL;
static OrtEnv *g_env = NULL;
#endif

void dt_ai_init()
{
#ifdef HAVE_ONNXRUNTIME
  if (g_ort == NULL) {
    g_ort = OrtGetApiBase()->GetApi(ORT_API_VERSION);
    if (!g_ort) {
      dt_print(DT_DEBUG_CONTROL, "[ai_core] Failed to get ONNX Runtime API\n");
      return;
    }
    // Create the environment
    if (g_ort->CreateEnv(ORT_LOGGING_LEVEL_WARNING, "darktable_ai", &g_env) != NULL) {
      dt_print(DT_DEBUG_CONTROL, "[ai_core] Failed to create ONNX environment\n");
      g_ort = NULL; // Mark as failed
    } else {
      dt_print(DT_DEBUG_CONTROL, "[ai_core] Initialized ONNX Runtime successfully\n");
    }
  }
#else
  dt_print(DT_DEBUG_CONTROL, "[ai_core] Compiled WITHOUT ONNX Runtime support\n");
#endif
}

void dt_ai_cleanup()
{
#ifdef HAVE_ONNXRUNTIME
  if (g_env) {
    g_ort->ReleaseEnv(g_env);
    g_env = NULL;
  }
#endif
}

gboolean dt_ai_is_available()
{
#ifdef HAVE_ONNXRUNTIME
  return (g_ort != NULL && g_env != NULL);
#else
  return FALSE;
#endif
}

dt_ai_model_t *dt_ai_load_model(const char *model_path, gboolean use_gpu)
{
  if (!dt_ai_is_available()) return NULL;

#ifdef HAVE_ONNXRUNTIME
  dt_ai_model_t *model = calloc(1, sizeof(dt_ai_model_t));
  OrtStatus *status;

  status = g_ort->CreateSessionOptions(&model->session_options);
  if (status != NULL) {
    dt_print(DT_DEBUG_CONTROL, "[ai_core] Failed to create session options\n");
    free(model);
    return NULL;
  }

  // TODO: Add CUDA/CoreML execution providers here based on use_gpu
  if (use_gpu) {
     // example: cleanup and append execution provider
  }

  status = g_ort->CreateSession(g_env, model_path, model->session_options, &model->session);
  if (status != NULL) {
    const char *msg = g_ort->GetErrorMessage(status);
    dt_print(DT_DEBUG_CONTROL, "[ai_core] Failed to load model %s: %s\n", model_path, msg);
    g_ort->ReleaseStatus(status);
    g_ort->ReleaseSessionOptions(model->session_options);
    free(model);
    return NULL;
  }

  status = g_ort->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &model->memory_info);
  if (status != NULL) {
    g_ort->ReleaseSession(model->session);
    g_ort->ReleaseSessionOptions(model->session_options);
    free(model);
    return NULL;
  }

  dt_print(DT_DEBUG_CONTROL, "[ai_core] Loaded model: %s\n", model_path);
  return model;
#else
  return NULL;
#endif
}

void dt_ai_unload_model(dt_ai_model_t *model)
{
  if (!model) return;

#ifdef HAVE_ONNXRUNTIME
  if (model->session) g_ort->ReleaseSession(model->session);
  if (model->session_options) g_ort->ReleaseSessionOptions(model->session_options);
  if (model->memory_info) g_ort->ReleaseMemoryInfo(model->memory_info);
#endif

  free(model);
}

gboolean dt_ai_run_inference(dt_ai_model_t *model,
                             dt_ai_tensor_t *inputs, size_t num_inputs,
                             dt_ai_tensor_t *outputs, size_t num_outputs)
{
  if (!model || !dt_ai_is_available()) return FALSE;

#ifdef HAVE_ONNXRUNTIME
  OrtStatus *status;
  OrtValue **input_tensors = calloc(num_inputs, sizeof(OrtValue *));
  const char **input_names = calloc(num_inputs, sizeof(char *));
  OrtValue **output_tensors = calloc(num_outputs, sizeof(OrtValue *));
  const char **output_names = calloc(num_outputs, sizeof(char *));

  // Prepare Inputs
  for(size_t i = 0; i < num_inputs; i++) {
    input_names[i] = inputs[i].name;
    status = g_ort->CreateTensorWithDataAsOrtValue(
      model->memory_info,
      inputs[i].data, inputs[i].data_size,
      inputs[i].shape, inputs[i].shape_len,
      ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT, // Simplified: assume float for now
      &input_tensors[i]
    );
    if (status != NULL) {
      dt_print(DT_DEBUG_CONTROL, "[ai_core] Failed to create input tensor %zu\n", i);
      goto cleanup; // error handling simplified
    }
  }

  // Prepare Outputs (pre-allocated buffer)
  for(size_t i = 0; i < num_outputs; i++) {
    output_names[i] = outputs[i].name;
    status = g_ort->CreateTensorWithDataAsOrtValue(
      model->memory_info,
      outputs[i].data, outputs[i].data_size,
      outputs[i].shape, outputs[i].shape_len,
      ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT, // Simplified
      &output_tensors[i]
    );
    if (status != NULL) {
      dt_print(DT_DEBUG_CONTROL, "[ai_core] Failed to create output tensor %zu\n", i);
      goto cleanup;
    }
  }

  // Run Inference
  status = g_ort->Run(
    model->session, NULL,
    input_names, (const OrtValue *const *)input_tensors, num_inputs,
    output_names, num_outputs,
    output_tensors
  );

  if (status != NULL) {
    const char *msg = g_ort->GetErrorMessage(status);
    dt_print(DT_DEBUG_CONTROL, "[ai_core] Inference failed: %s\n", msg);
    g_ort->ReleaseStatus(status);
    goto cleanup; // return FALSE
  }

  // Success
  // Resources are released below, buffers are owned by caller
  free(input_tensors);
  free(input_names);
  free(output_tensors);
  free(output_names);
  return TRUE;

cleanup:
  free(input_tensors);
  free(input_names);
  free(output_tensors);
  free(output_names);
  return FALSE;

#else
  return FALSE;
#endif
}
