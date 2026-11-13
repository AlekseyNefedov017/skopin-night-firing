#pragma once

namespace skopin {

// Полезная высота горна. Под самое нижнее изделие полку не ставят:
// полка добавляется только под каждое следующее.
inline constexpr int kKilnHeightCm = 48;

inline constexpr int kShelfCm = 2;

// Стенка толще этого на огне трескается. С высотой не сравнивать:
// толщина в миллиметрах, высота в сантиметрах.
inline constexpr int kCrackThicknessMm = 12;

}  // namespace skopin
