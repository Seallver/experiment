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

# ========== 2. 提取水印 ==========
def extract_watermark(watermarked_path, original_path, output_path, wm_shape, attacked_method='none'):
    if attacked_method == 'flip':
        wm_img = cv2.flip(cv2.imread(watermarked_path, cv2.IMREAD_GRAYSCALE), 1)
    else:
        wm_img = cv2.imread(watermarked_path, cv2.IMREAD_GRAYSCALE)
        
    ori_img = cv2.imread(original_path, cv2.IMREAD_GRAYSCALE)

    cA_wm, _ = pywt.dwt2(wm_img, 'haar')
    cA_ori, _ = pywt.dwt2(ori_img, 'haar')

    alpha = 0.5
    wm_extracted = (cA_wm - cA_ori) / alpha
    wm_extracted = wm_extracted[:wm_shape[0], :wm_shape[1]]
    wm_extracted = np.clip(wm_extracted, 0, 255).astype(np.uint8)

    cv2.imwrite(output_path, wm_extracted)
    print(f"* 水印提取完成，保存为：{output_path}\n")

# ========== 3. 鲁棒性测试 ==========
def attack_image(image_path, output_path, method='contrast'):
    image = cv2.imread(image_path)

    if method == 'flip':
        attacked = cv2.flip(image, 1)
    elif method == 'translate':
        M = np.float32([[1, 0, 50], [0, 1, 50]])
        attacked = cv2.warpAffine(image, M, (image.shape[1], image.shape[0]))
    elif method == 'crop':
        h, w = image.shape[:2]
        crop_margin = 100 
        # 防止裁剪超出边界
        crop_margin = min(crop_margin, h//2 - 1, w//2 - 1)
        attacked = image[crop_margin:h-crop_margin, crop_margin:w-crop_margin]
        attacked = cv2.resize(attacked, (w, h))
    elif method == 'contrast':
        attacked = cv2.convertScaleAbs(image, alpha=1.5, beta=0)
    else:
        return

    cv2.imwrite(output_path, attacked)
    print(f"* 攻击方法 '{method}' 已应用，保存为：{output_path}")


# ========== 4. 主函数 ==========
if __name__ == '__main__':
    # 输入路径
    original_img = 'image.png'
    watermark_img = 'logo.png'
    wm_out_img = 'watermarked.png'

    # 1. 嵌入水印
    wm_shape = embed_watermark(original_img, watermark_img, wm_out_img)

    # 2. 攻击测试
    attack_methods = ['flip', 'translate', 'crop', 'contrast']
    for method in attack_methods:
        attacked_img = f"attacked_{method}.png"
        attack_image(wm_out_img, attacked_img, method=method)

        # 3. 提取水印
        extracted_img = f"extracted_{method}.png"
        extract_watermark(attacked_img, original_img, extracted_img, wm_shape, attacked_method=method)

