# Project2 编程实现图片水印嵌入和提取，并进行鲁棒性测试

一个基于 C 语言实现的图像水印嵌入与提取系统，支持 24 位 BMP 图像，直接通过最低有效位（LSB）嵌入文字水印，并具备鲁棒性图像处理功能，如翻转、平移、裁剪与对比度调整。

---

##  实验要求

-  **水印嵌入**：通过修改像素蓝色通道的最低有效位嵌入字符串；
-  **水印提取**：从嵌入图中还原原始水印内容；
-  **鲁棒性测试**：
  - 水平翻转
  - 垂直翻转
  - 像素平移
  - 图像裁剪
  - 对比度增强

---

##  项目结构

```

.
├── main.c              # 主程序：嵌入与提取流程
├── image\_utils.c/h     # BMP 图像读写与内存管理
├── robust.c/h          # 图像鲁棒性处理函数
├── original.bmp        # 示例输入图像
├── output/             # 输出图像目录
│   ├── synthesis.bmp
│   ├── extract_watermark.bmp
│   └── extrac_background.bmp

````

##  使用说明

```bash
./watermark
```

默认操作流程：

1. 加载输入图像 `original.bmp`
2. 嵌入水印图像 `"watermark.bmp"`
3. 生成嵌入水印后的图像 `synthesis.bmp`
4. 对图像进行一系列鲁棒性变换：

   * `flip_h.bmp`：水平翻转
   * `flip_v.bmp`：垂直翻转
   * `shifted.bmp`：平移（+5, +5）
   * `cropped.bmp`：裁剪边缘 10 像素
   * `contrast.bmp`：对比度增强（x1.5）
5. 从最终图像中尝试提取水印内容

---

##  水印嵌入示例

嵌入山东大学图案到原始图像的像素最低位：
> ![屏幕截图 2025-07-10 173311](https://github.com/user-attachments/assets/a96eeb8e-d4c3-4069-bc9b-c42b5ac09b23)


> 原图：
> ![屏幕截图 2025-07-10 172831](https://github.com/user-attachments/assets/fc9d9a18-2bb1-4c85-9dc9-bb1a8064ef4a)


> 嵌入后图像：
> ![屏幕截图 2025-07-10 173335](https://github.com/user-attachments/assets/7d2baf45-0eea-4643-aeac-03bffc218c35)


嵌入后图像肉眼无差异，嵌入信息藏于像素蓝色通道 LSB。

---

##  提取效果示例

程序将输出：

```
Extracted Successful!
```

---

##  核心逻辑简析

### 水印嵌入（LSB）

```c
img->data[i][j].b = (img->data[i][j].b & 0xFE) | bit;
```

将字符按位分解，嵌入蓝色通道最低位。

### 水印提取

```c
result[k / 8] <<= 1;
result[k / 8] |= (img->data[i][j].b & 1);
```

按顺序还原嵌入的图片数据。

---

##  鲁棒性测试功能

| 操作类型      | 函数名              
| --------- | ----------------- | 
| 水平翻转      | `flip_horizontal` |
| 垂直翻转      | `flip_vertical`   | 
| 平移（dx,dy） | `shift_image`     | 
| 裁剪        | `crop_image`      | 
| 对比度增强     | `adjust_contrast` |

> 示例对比图（原图  vs 提取水印）

| 原图| 嵌入水印提取结果  |

![屏幕截图 2025-07-10 172522](https://github.com/user-attachments/assets/4ee42c49-eb58-4458-8c68-4df587e0c07b)

---

