#if 1 // 改为 0 可禁用本文件（同时只能启用一个 user*.cpp）

#include "eyes_common.h"

// 本文件提供在眼睛动画中“插入”用户代码的简单方式，
// 无需维护多个眼睛代码分支。只需修改本文件内容，编译烧录即可。

// 用户全局变量可在此声明，建议用 static，例如：
// static int foo = 42;

// 在 setup() 末尾附近调用一次
void user_setup(void) {
}

// 在眼睛动画期间周期性调用。
// 在最后一只眼开始绘制前调用，以减轻画面撕裂。
// 本函数会阻塞，不与眼睛动画并行；此处耗时直接影响帧率，
// 请保持简单。避免循环（如舵机/NeoPixel 动画），改用状态机。
// 调用间隔不恒定（每帧渲染时间不同），动画请用 millis()/micros()
// 按 elapsed 时间计算，而非简单累加。
void user_loop(void) {
/*
  假设有全局 bool animating（表示正在运动），
  以及 uint32_t startTime（触发时刻）和 transitionTime（总时长，微秒）。
  可能是舵机、NeoPixel 等。伪代码示例：

  if(!animating) {
    未在运动，检测传感器触发...
    if(读取到某传感器) {
      触发运动！记录 startTime，设置 transitionTime = 1.5 秒，animating = true
      本帧尚不移动，下一帧才开始。
    }
  } else {
    正在运动，忽略触发，执行移动...
    uint32_t elapsed = millis() - startTime;
    if(elapsed < transitionTime) {
      运动进行中... 进度 ratio = elapsed / transitionTime (0.0~1.0)
      根据 ratio 更新输出
    } else {
      运动结束，归位并清除 animating = false
    }
  }
*/
}

#endif
