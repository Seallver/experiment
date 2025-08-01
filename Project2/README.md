# 项目介绍
本项目实现了一个简单的**数字图像水印嵌入与提取系统**，并对其在常见图像攻击下的鲁棒性进行了测试。

## 项目功能
- 将 logo 图像嵌入至原始图片中形成水印图；
- 支持以下常见攻击的水印鲁棒性测试：
  - 水平翻转（flip）
  - 平移（translate）
  - 裁剪（crop）
  - 提高对比度（contrast）
- 支持从受攻击图像中恢复出水印图案。
## 实现原理

本项目采用 **离散小波变换（Discrete Wavelet Transform, DWT）** 方法，实现图像数字水印的嵌入与提取。整个流程分为三个阶段：水印嵌入、攻击模拟、水印提取。

---

### 1. 水印嵌入（Embedding）

1. 将原图像 $I$ 和水印图像 $W$ 转换为灰度图；
2. 对图像 $I$ 进行二维 Haar 小波分解（1 级 DWT），得到四个子带：

$$
\text{DWT}(I) \Rightarrow \{cA, cH, cV, cD\}
$$

   其中：
   - $cA$：低频近似系数（Approximation）；
   - $cH, cV, cD$：水平、垂直与对角方向的高频细节系数。
3. 将水印图像 $W$ 缩放为 $cA$ 尺寸的 $\frac{1}{4}$即：

$$
\frac{m}{2} \times \frac{n}{2}
$$

4. 以加性嵌入方式将水印嵌入 $cA$ 的左上角区域：

$$
cA_{\text{embed}}(i,j) = cA(i,j) + \alpha \cdot W(i,j), \quad \forall (i,j) \in \left[0,\tfrac{m}{2}\right) \times \left[0,\tfrac{n}{2}\right)
$$

   其中 $\alpha$ 为嵌入强度因子；
   
5. 使用修改后的 $cA_{\text{embed}}$ 与原始的高频系数 $cH, cV, cD$ 进行逆小波变换（IDWT），重构出带水印图像 $I_w$：

$$
I_w = \text{IDWT}(cA_{\text{embed}}, cH, cV, cD)
$$

---

### 2. 攻击模拟（Attacks）

为了测试水印的鲁棒性，对水印图像 $I_w$ 施加多种常见攻击，包括：
- 图像翻转（Flip）
- 图像裁剪（Cropping）
- 图像平移（Translation）
- 对比度调节（Contrast Adjustment）

攻击后的图像记为 $I_w^{\text{attacked}}$。

---

### 3. 水印提取（Extraction）

1. 对原图像 $I$ 和攻击图像 $I_w^{\text{attacked}}$ 分别做 DWT 分解，提取各自的近似系数：

$$
\text{DWT}(I) \Rightarrow cA, \quad \text{DWT}(I_w^{\text{attacked}}) \Rightarrow cA^{\text{attacked}}
$$

2. 计算差值并恢复水印图像：

$$
\hat{W}(i,j) = \frac{cA^{\text{attacked}}(i,j) - cA(i,j)}{\alpha}, \quad \forall (i,j) \in \left[0,\tfrac{m}{2}\right) \times \left[0,\tfrac{n}{2}\right)
$$
   
3. 若有需要，可对 $\hat{W}$ 进行插值放大以便展示或评估。

---

**备注：**
- 嵌入强度因子 $\alpha$ 控制水印可见性与鲁棒性之间的权衡；
- 仅将水印嵌入低频 $cA$ 区域有助于在常规图像处理下保持水印稳定。


## 项目结构

```bash
.
├── README.md              # 本说明文档
├── watermark.py           # 主程序
├── image.png              # 原始图像
└── logo.png               # 水印图像
```

# 使用说明

环境依赖：
- Python `(v3.10.16)`
- Python 包依赖：
   - `numpy`
   - `pywt`
   - `cv2`

执行程序：
```bash
python3 watermark.py
```
运行后会在`output`文件夹中生成：
- `watermarked.png`：嵌入水印后的图像
- `attacked_*.png`：经过攻击处理的图像
- `extracted_*.png`：从被攻击图像中提取的水印图像

注意事项：
- 水印在图像左上角，filp处理后的在右上角
- 通过调整 $\alpha$ 的大小，可以控制水印的可见性，该值越大，水印越明显，为了使实验效果明显，本项目默认 $\alpha=0.3$，根据实际要求，如果追求更好的隐蔽性，可以降低 $\alpha$ 的值