import cv2
import numpy as np
import pywt
import os

# ========== 1. 嵌入水印 ==========
def embed_watermark(image_path, watermark_path, output_path):
    image = cv2.imread(image_path, cv2.IMREAD_GRAYSCALE)
    watermark = cv2.imread(watermark_path, cv2.IMREAD_GRAYSCALE)

    # 调整大小
    wm_small = cv2.resize(watermark, (image.shape[1] // 4, image.shape[0] // 4))

    # DWT 分解
    coeffs = pywt.dwt2(image, 'haar')
    cA, (cH, cV, cD) = coeffs

    # 嵌入水印到 cA
    alpha = 0.3
    rows, cols = wm_small.shape
    cA[:rows, :cols] += alpha * wm_small

    # 重建图像
    watermarked = pywt.idwt2((cA, (cH, cV, cD)), 'haar')
    watermarked = np.uint8(np.clip(watermarked, 0, 255))

    cv2.imwrite(output_path, watermarked)
    print(f"水印嵌入成功，保存为：{output_path}\n")
    return wm_small.shape



