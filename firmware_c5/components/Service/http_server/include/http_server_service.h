// Copyright (c) 2025 HIGH CODE LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef HTTP_SERVER_SERVICE_H
#define HTTP_SERVER_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include "esp_http_server.h"

typedef enum {
  HTTP_STATUS_OK_200 = 200,
  HTTP_STATUS_CREATED_201 = 201,
  HTTP_STATUS_BAD_REQUEST_400 = 400,
  HTTP_STATUS_UNAUTHORIZED_401 = 401,
  HTTP_STATUS_FORBIDDEN_403 = 403,
  HTTP_STATUS_NOT_FOUND_404 = 404,
  HTTP_STATUS_REQUEST_TIMEOUT_408 = 408,
  HTTP_STATUS_INTERNAL_ERROR_500 = 500,
} http_status_t;

/**
 * @brief Start the HTTP web server.
 *
 * @return ESP_OK on success, ESP_FAIL if the server could not start.
 */
esp_err_t start_web_server(void);

/**
 * @brief Stop the HTTP web server.
 *
 * @return ESP_OK on success.
 */
esp_err_t stop_web_server(void);

/**
 * @brief Register a URI handler with the running server.
 *
 * @param uri_handler Pointer to the URI handler descriptor.
 * @return ESP_OK on success, ESP_FAIL if the server is not started.
 */
esp_err_t http_service_register_uri(const httpd_uri_t *uri_handler);

/**
 * @brief Stop the HTTP server (alias).
 *
 * @return ESP_OK on success.
 */
esp_err_t stop_http_server(void);

/**
 * @brief Send an HTTP error response.
 *
 * @param req     The HTTP request handle.
 * @param status_code HTTP status code to send.
 * @param msg     Error message string.
 * @return ESP_OK on success.
 */
esp_err_t http_service_send_error(httpd_req_t *req, http_status_t status_code, const char *msg);

/**
 * @brief Receive the request body into a buffer.
 *
 * @param req         The HTTP request handle.
 * @param buffer      Destination buffer.
 * @param buffer_size Size of the destination buffer.
 * @return ESP_OK on success.
 */
esp_err_t http_service_req_recv(httpd_req_t *req, char *buffer, size_t buffer_size);

/**
 * @brief Extract a key-value pair from a query string.
 *
 * @param data_buffer The query string buffer.
 * @param key         Key to search for.
 * @param out_val     Buffer to receive the value.
 * @param out_size    Size of the output buffer.
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if key missing.
 */
esp_err_t http_service_query_key_value(const char *data_buffer,
                                       const char *key,
                                       char *out_val,
                                       size_t out_size);

/**
 * @brief Send an HTTP response with the given buffer.
 *
 * @param req    The HTTP request handle.
 * @param buffer Response body.
 * @param length Length of the response, or HTTPD_RESP_USE_STRLEN.
 * @return ESP_OK on success.
 */
esp_err_t http_service_send_response(httpd_req_t *req, const char *buffer, ssize_t length);

/**
 * @brief Send a file from the SD card as the HTTP response.
 *
 * @param req      The HTTP request handle.
 * @param filepath Path to the file on the SD card.
 * @return ESP_OK on success.
 */
esp_err_t http_service_send_file_from_sd(httpd_req_t *req, const char *filepath);

/**
 * @brief Load a file into a heap-allocated buffer.
 *
 * @param path Path to the file.
 * @return Pointer to the buffer (caller must free), or NULL on error.
 */
const char *get_html_buffer(const char *path);

#ifdef __cplusplus
}
#endif

#endif // HTTP_SERVER_SERVICE_H
