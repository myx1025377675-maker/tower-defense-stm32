## 目标

在不把全字库编译进程序（避免下载变慢）的前提下，让工程能按需显示中文。

做法：把 `.FON` 字库文件放在工程里（不参与编译），需要新增汉字时，从 `.FON` 中抽取对应字模，自动生成一个很小的 `e08_fon_subset.c/h`，再编译下载。

## 字库材料放置

把下面两个文件放到本工程的 `fonts/` 目录（已放好即可跳过）：

- `fonts/GB2312_16.FON`
- `fonts/GB2312_32.FON`

## 新增汉字（推荐方式）

在工程根目录打开命令行，执行：

```bash
python tools/make_subset_from_text.py --text "这里填你要新增显示的中文"
```

执行完成后，会自动更新并生成：

- `Drivers/User/Src/e08_fon_subset.c`
- `Drivers/User/Inc/e08_fon_subset.h`

然后在 Keil 里 `Rebuild` + `Download` 即可。

## 说明

- 这个流程只覆盖 **GB2312** 字符集里的汉字（课程配套 `.FON` 也是 GB2312）。
- ASCII（英文/数字）不依赖 `.FON`，例程自带点阵表。

