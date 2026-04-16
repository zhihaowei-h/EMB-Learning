/*===============================================
 *   文件名称：cloud_upload.h
 *   创 建 者：IoT Gateway System
 *   创建日期：2026年04月16日
 *   描    述：云平台数据上传模块
 *            使用HTTP协议上传到EmbSky云平台
 ================================================*/

#ifndef __CLOUD_UPLOAD_H
#define __CLOUD_UPLOAD_H

#include "iot_gateway.h"

/*
 * 功能：上传多个传感器数据到云平台
 * 参数：data - 传感器数据数组
 *       count - 传感器数量
 * 返回：成功返回0，失败返回-1
 */
int cloud_upload_sensor_datas(sensor_data_t *data, int count);

/*
 * 功能：上传单个传感器数据到云平台
 * 参数：sensor_id - 传感器ID
 *       data_value - 数据值
 * 返回：成功返回0，失败返回-1
 */
int cloud_upload_single_data(int sensor_id, double data_value);

#endif /* __CLOUD_UPLOAD_H */
