# C++17 Hot100：完整原创学习轨道

当前官方正式范围：100题、17类；LC695为额外公开练习，单列1题。100份源码均独立成文件，逐题有中文思考、状态与步骤、自建例子、正确性解释、复杂度、C++风险、追问和练习。

先读[学习路线与模式](PATTERNS.md)，再看[官方题单与实现边界](SOURCES.md)、[实现复用提醒](ERRATA.md)和[验证方法](TESTING.md)。官方题单核对日期2026-10-10。

## 如何用本地代码

仓库根目录运行 `python3 algorithms/cpp/validate.py` 可编译运行全部正式题与补充题；运行 `python3 algorithms/cpp/validate.py --sanitizers` 加入UBSan和ASan检查。在线判题未提交，不宣称在线AC。每题也给出单独编译命令。

本地支持头 [support.hpp](../../algorithms/cpp/support.hpp) 提供STL、节点类型和测试所有权工具；它不是必须粘贴到在线平台的额外题解。

本地目标明确要求32位有符号 `int` 的值域、8位字节和ASCII字母/数字；`long long` 至少63个值位。C++17本身不保证前三项，支持头会逐项断言，详见[表示契约](SOURCES.md)。移植验证与语言版本标签要分别说明。

LC53的分治与LC295的全范围/99%范围追问另有[独立练习源码](../../algorithms/cpp/exercises/followup_variants.hpp)和[可执行回归](../../algorithms/cpp/exercises/test_followup_variants.cpp)，运行方法见[验证方法](TESTING.md)。它们不增加正式或补充题数。

## 正式100题

### 01. 哈希（3题）

- [1. 两数之和](problems/0001-two-sum.md) · EASY · [源码](../../algorithms/cpp/solutions/p0001_two_sum.hpp) · [测试](../../algorithms/cpp/tests/test_p0001.cpp)
- [49. 字母异位词分组](problems/0049-group-anagrams.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0049_group_anagrams.hpp) · [测试](../../algorithms/cpp/tests/test_p0049.cpp)
- [128. 最长连续序列](problems/0128-longest-consecutive-sequence.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0128_longest_consecutive_sequence.hpp) · [测试](../../algorithms/cpp/tests/test_p0128.cpp)

### 02. 双指针（4题）

- [283. 移动零](problems/0283-move-zeroes.md) · EASY · [源码](../../algorithms/cpp/solutions/p0283_move_zeroes.hpp) · [测试](../../algorithms/cpp/tests/test_p0283.cpp)
- [11. 盛最多水的容器](problems/0011-container-with-most-water.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0011_container_with_most_water.hpp) · [测试](../../algorithms/cpp/tests/test_p0011.cpp)
- [15. 三数之和](problems/0015-3sum.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0015_3sum.hpp) · [测试](../../algorithms/cpp/tests/test_p0015.cpp)
- [42. 接雨水](problems/0042-trapping-rain-water.md) · HARD · [源码](../../algorithms/cpp/solutions/p0042_trapping_rain_water.hpp) · [测试](../../algorithms/cpp/tests/test_p0042.cpp)

### 03. 滑动窗口（2题）

- [3. 无重复字符的最长子串](problems/0003-longest-substring-without-repeating-characters.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0003_longest_substring_without_repeating_characters.hpp) · [测试](../../algorithms/cpp/tests/test_p0003.cpp)
- [438. 找到字符串中所有字母异位词](problems/0438-find-all-anagrams-in-a-string.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0438_find_all_anagrams_in_a_string.hpp) · [测试](../../algorithms/cpp/tests/test_p0438.cpp)

### 04. 子串（3题）

- [560. 和为 K 的子数组](problems/0560-subarray-sum-equals-k.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0560_subarray_sum_equals_k.hpp) · [测试](../../algorithms/cpp/tests/test_p0560.cpp)
- [239. 滑动窗口最大值](problems/0239-sliding-window-maximum.md) · HARD · [源码](../../algorithms/cpp/solutions/p0239_sliding_window_maximum.hpp) · [测试](../../algorithms/cpp/tests/test_p0239.cpp)
- [76. 最小覆盖子串](problems/0076-minimum-window-substring.md) · HARD · [源码](../../algorithms/cpp/solutions/p0076_minimum_window_substring.hpp) · [测试](../../algorithms/cpp/tests/test_p0076.cpp)

### 05. 普通数组（5题）

- [53. 最大子数组和](problems/0053-maximum-subarray.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0053_maximum_subarray.hpp) · [测试](../../algorithms/cpp/tests/test_p0053.cpp)
- [56. 合并区间](problems/0056-merge-intervals.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0056_merge_intervals.hpp) · [测试](../../algorithms/cpp/tests/test_p0056.cpp)
- [189. 轮转数组](problems/0189-rotate-array.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0189_rotate_array.hpp) · [测试](../../algorithms/cpp/tests/test_p0189.cpp)
- [238. 除了自身以外数组的乘积](problems/0238-product-of-array-except-self.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0238_product_of_array_except_self.hpp) · [测试](../../algorithms/cpp/tests/test_p0238.cpp)
- [41. 缺失的第一个正数](problems/0041-first-missing-positive.md) · HARD · [源码](../../algorithms/cpp/solutions/p0041_first_missing_positive.hpp) · [测试](../../algorithms/cpp/tests/test_p0041.cpp)

### 06. 矩阵（4题）

- [73. 矩阵置零](problems/0073-set-matrix-zeroes.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0073_set_matrix_zeroes.hpp) · [测试](../../algorithms/cpp/tests/test_p0073.cpp)
- [54. 螺旋矩阵](problems/0054-spiral-matrix.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0054_spiral_matrix.hpp) · [测试](../../algorithms/cpp/tests/test_p0054.cpp)
- [48. 旋转图像](problems/0048-rotate-image.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0048_rotate_image.hpp) · [测试](../../algorithms/cpp/tests/test_p0048.cpp)
- [240. 搜索二维矩阵 II](problems/0240-search-a-2d-matrix-ii.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0240_search_a_2d_matrix_ii.hpp) · [测试](../../algorithms/cpp/tests/test_p0240.cpp)

### 07. 链表（14题）

- [160. 相交链表](problems/0160-intersection-of-two-linked-lists.md) · EASY · [源码](../../algorithms/cpp/solutions/p0160_intersection_of_two_linked_lists.hpp) · [测试](../../algorithms/cpp/tests/test_p0160.cpp)
- [206. 反转链表](problems/0206-reverse-linked-list.md) · EASY · [源码](../../algorithms/cpp/solutions/p0206_reverse_linked_list.hpp) · [测试](../../algorithms/cpp/tests/test_p0206.cpp)
- [234. 回文链表](problems/0234-palindrome-linked-list.md) · EASY · [源码](../../algorithms/cpp/solutions/p0234_palindrome_linked_list.hpp) · [测试](../../algorithms/cpp/tests/test_p0234.cpp)
- [141. 环形链表](problems/0141-linked-list-cycle.md) · EASY · [源码](../../algorithms/cpp/solutions/p0141_linked_list_cycle.hpp) · [测试](../../algorithms/cpp/tests/test_p0141.cpp)
- [142. 环形链表 II](problems/0142-linked-list-cycle-ii.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0142_linked_list_cycle_ii.hpp) · [测试](../../algorithms/cpp/tests/test_p0142.cpp)
- [21. 合并两个有序链表](problems/0021-merge-two-sorted-lists.md) · EASY · [源码](../../algorithms/cpp/solutions/p0021_merge_two_sorted_lists.hpp) · [测试](../../algorithms/cpp/tests/test_p0021.cpp)
- [2. 两数相加](problems/0002-add-two-numbers.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0002_add_two_numbers.hpp) · [测试](../../algorithms/cpp/tests/test_p0002.cpp)
- [19. 删除链表的倒数第 N 个结点](problems/0019-remove-nth-node-from-end-of-list.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0019_remove_nth_node_from_end_of_list.hpp) · [测试](../../algorithms/cpp/tests/test_p0019.cpp)
- [24. 两两交换链表中的节点](problems/0024-swap-nodes-in-pairs.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0024_swap_nodes_in_pairs.hpp) · [测试](../../algorithms/cpp/tests/test_p0024.cpp)
- [25. K 个一组翻转链表](problems/0025-reverse-nodes-in-k-group.md) · HARD · [源码](../../algorithms/cpp/solutions/p0025_reverse_nodes_in_k_group.hpp) · [测试](../../algorithms/cpp/tests/test_p0025.cpp)
- [138. 随机链表的复制](problems/0138-copy-list-with-random-pointer.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0138_copy_list_with_random_pointer.hpp) · [测试](../../algorithms/cpp/tests/test_p0138.cpp)
- [148. 排序链表](problems/0148-sort-list.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0148_sort_list.hpp) · [测试](../../algorithms/cpp/tests/test_p0148.cpp)
- [23. 合并 K 个升序链表](problems/0023-merge-k-sorted-lists.md) · HARD · [源码](../../algorithms/cpp/solutions/p0023_merge_k_sorted_lists.hpp) · [测试](../../algorithms/cpp/tests/test_p0023.cpp)
- [146. LRU 缓存](problems/0146-lru-cache.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0146_lru_cache.hpp) · [测试](../../algorithms/cpp/tests/test_p0146.cpp)

### 08. 二叉树（15题）

- [94. 二叉树的中序遍历](problems/0094-binary-tree-inorder-traversal.md) · EASY · [源码](../../algorithms/cpp/solutions/p0094_binary_tree_inorder_traversal.hpp) · [测试](../../algorithms/cpp/tests/test_p0094.cpp)
- [104. 二叉树的最大深度](problems/0104-maximum-depth-of-binary-tree.md) · EASY · [源码](../../algorithms/cpp/solutions/p0104_maximum_depth_of_binary_tree.hpp) · [测试](../../algorithms/cpp/tests/test_p0104.cpp)
- [226. 翻转二叉树](problems/0226-invert-binary-tree.md) · EASY · [源码](../../algorithms/cpp/solutions/p0226_invert_binary_tree.hpp) · [测试](../../algorithms/cpp/tests/test_p0226.cpp)
- [101. 对称二叉树](problems/0101-symmetric-tree.md) · EASY · [源码](../../algorithms/cpp/solutions/p0101_symmetric_tree.hpp) · [测试](../../algorithms/cpp/tests/test_p0101.cpp)
- [543. 二叉树的直径](problems/0543-diameter-of-binary-tree.md) · EASY · [源码](../../algorithms/cpp/solutions/p0543_diameter_of_binary_tree.hpp) · [测试](../../algorithms/cpp/tests/test_p0543.cpp)
- [102. 二叉树的层序遍历](problems/0102-binary-tree-level-order-traversal.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0102_binary_tree_level_order_traversal.hpp) · [测试](../../algorithms/cpp/tests/test_p0102.cpp)
- [108. 将有序数组转换为二叉搜索树](problems/0108-convert-sorted-array-to-binary-search-tree.md) · EASY · [源码](../../algorithms/cpp/solutions/p0108_convert_sorted_array_to_binary_search_tree.hpp) · [测试](../../algorithms/cpp/tests/test_p0108.cpp)
- [98. 验证二叉搜索树](problems/0098-validate-binary-search-tree.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0098_validate_binary_search_tree.hpp) · [测试](../../algorithms/cpp/tests/test_p0098.cpp)
- [230. 二叉搜索树中第 K 小的元素](problems/0230-kth-smallest-element-in-a-bst.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0230_kth_smallest_element_in_a_bst.hpp) · [测试](../../algorithms/cpp/tests/test_p0230.cpp)
- [199. 二叉树的右视图](problems/0199-binary-tree-right-side-view.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0199_binary_tree_right_side_view.hpp) · [测试](../../algorithms/cpp/tests/test_p0199.cpp)
- [114. 二叉树展开为链表](problems/0114-flatten-binary-tree-to-linked-list.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0114_flatten_binary_tree_to_linked_list.hpp) · [测试](../../algorithms/cpp/tests/test_p0114.cpp)
- [105. 从前序与中序遍历序列构造二叉树](problems/0105-construct-binary-tree-from-preorder-and-inorder-traversal.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0105_construct_binary_tree_from_preorder_and_inorder_traversal.hpp) · [测试](../../algorithms/cpp/tests/test_p0105.cpp)
- [437. 路径总和 III](problems/0437-path-sum-iii.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0437_path_sum_iii.hpp) · [测试](../../algorithms/cpp/tests/test_p0437.cpp)
- [236. 二叉树的最近公共祖先](problems/0236-lowest-common-ancestor-of-a-binary-tree.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0236_lowest_common_ancestor_of_a_binary_tree.hpp) · [测试](../../algorithms/cpp/tests/test_p0236.cpp)
- [124. 二叉树中的最大路径和](problems/0124-binary-tree-maximum-path-sum.md) · HARD · [源码](../../algorithms/cpp/solutions/p0124_binary_tree_maximum_path_sum.hpp) · [测试](../../algorithms/cpp/tests/test_p0124.cpp)

### 09. 图论（4题）

- [200. 岛屿数量](problems/0200-number-of-islands.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0200_number_of_islands.hpp) · [测试](../../algorithms/cpp/tests/test_p0200.cpp)
- [994. 腐烂的橘子](problems/0994-rotting-oranges.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0994_rotting_oranges.hpp) · [测试](../../algorithms/cpp/tests/test_p0994.cpp)
- [207. 课程表](problems/0207-course-schedule.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0207_course_schedule.hpp) · [测试](../../algorithms/cpp/tests/test_p0207.cpp)
- [208. 实现 Trie (前缀树)](problems/0208-implement-trie-prefix-tree.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0208_implement_trie_prefix_tree.hpp) · [测试](../../algorithms/cpp/tests/test_p0208.cpp)

### 10. 回溯（8题）

- [46. 全排列](problems/0046-permutations.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0046_permutations.hpp) · [测试](../../algorithms/cpp/tests/test_p0046.cpp)
- [78. 子集](problems/0078-subsets.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0078_subsets.hpp) · [测试](../../algorithms/cpp/tests/test_p0078.cpp)
- [17. 电话号码的字母组合](problems/0017-letter-combinations-of-a-phone-number.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0017_letter_combinations_of_a_phone_number.hpp) · [测试](../../algorithms/cpp/tests/test_p0017.cpp)
- [39. 组合总和](problems/0039-combination-sum.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0039_combination_sum.hpp) · [测试](../../algorithms/cpp/tests/test_p0039.cpp)
- [22. 括号生成](problems/0022-generate-parentheses.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0022_generate_parentheses.hpp) · [测试](../../algorithms/cpp/tests/test_p0022.cpp)
- [79. 单词搜索](problems/0079-word-search.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0079_word_search.hpp) · [测试](../../algorithms/cpp/tests/test_p0079.cpp)
- [131. 分割回文串](problems/0131-palindrome-partitioning.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0131_palindrome_partitioning.hpp) · [测试](../../algorithms/cpp/tests/test_p0131.cpp)
- [51. N 皇后](problems/0051-n-queens.md) · HARD · [源码](../../algorithms/cpp/solutions/p0051_n_queens.hpp) · [测试](../../algorithms/cpp/tests/test_p0051.cpp)

### 11. 二分查找（6题）

- [35. 搜索插入位置](problems/0035-search-insert-position.md) · EASY · [源码](../../algorithms/cpp/solutions/p0035_search_insert_position.hpp) · [测试](../../algorithms/cpp/tests/test_p0035.cpp)
- [74. 搜索二维矩阵](problems/0074-search-a-2d-matrix.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0074_search_a_2d_matrix.hpp) · [测试](../../algorithms/cpp/tests/test_p0074.cpp)
- [34. 在排序数组中查找元素的第一个和最后一个位置](problems/0034-find-first-and-last-position-of-element-in-sorted-array.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0034_find_first_and_last_position_of_element_in_sorted_array.hpp) · [测试](../../algorithms/cpp/tests/test_p0034.cpp)
- [33. 搜索旋转排序数组](problems/0033-search-in-rotated-sorted-array.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0033_search_in_rotated_sorted_array.hpp) · [测试](../../algorithms/cpp/tests/test_p0033.cpp)
- [153. 寻找旋转排序数组中的最小值](problems/0153-find-minimum-in-rotated-sorted-array.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0153_find_minimum_in_rotated_sorted_array.hpp) · [测试](../../algorithms/cpp/tests/test_p0153.cpp)
- [4. 寻找两个正序数组的中位数](problems/0004-median-of-two-sorted-arrays.md) · HARD · [源码](../../algorithms/cpp/solutions/p0004_median_of_two_sorted_arrays.hpp) · [测试](../../algorithms/cpp/tests/test_p0004.cpp)

### 12. 栈（5题）

- [20. 有效的括号](problems/0020-valid-parentheses.md) · EASY · [源码](../../algorithms/cpp/solutions/p0020_valid_parentheses.hpp) · [测试](../../algorithms/cpp/tests/test_p0020.cpp)
- [155. 最小栈](problems/0155-min-stack.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0155_min_stack.hpp) · [测试](../../algorithms/cpp/tests/test_p0155.cpp)
- [394. 字符串解码](problems/0394-decode-string.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0394_decode_string.hpp) · [测试](../../algorithms/cpp/tests/test_p0394.cpp)
- [739. 每日温度](problems/0739-daily-temperatures.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0739_daily_temperatures.hpp) · [测试](../../algorithms/cpp/tests/test_p0739.cpp)
- [84. 柱状图中最大的矩形](problems/0084-largest-rectangle-in-histogram.md) · HARD · [源码](../../algorithms/cpp/solutions/p0084_largest_rectangle_in_histogram.hpp) · [测试](../../algorithms/cpp/tests/test_p0084.cpp)

### 13. 堆（3题）

- [215. 数组中的第K个最大元素](problems/0215-kth-largest-element-in-an-array.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0215_kth_largest_element_in_an_array.hpp) · [测试](../../algorithms/cpp/tests/test_p0215.cpp)
- [347. 前 K 个高频元素](problems/0347-top-k-frequent-elements.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0347_top_k_frequent_elements.hpp) · [测试](../../algorithms/cpp/tests/test_p0347.cpp)
- [295. 数据流的中位数](problems/0295-find-median-from-data-stream.md) · HARD · [源码](../../algorithms/cpp/solutions/p0295_find_median_from_data_stream.hpp) · [测试](../../algorithms/cpp/tests/test_p0295.cpp)

### 14. 贪心算法（4题）

- [121. 买卖股票的最佳时机](problems/0121-best-time-to-buy-and-sell-stock.md) · EASY · [源码](../../algorithms/cpp/solutions/p0121_best_time_to_buy_and_sell_stock.hpp) · [测试](../../algorithms/cpp/tests/test_p0121.cpp)
- [55. 跳跃游戏](problems/0055-jump-game.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0055_jump_game.hpp) · [测试](../../algorithms/cpp/tests/test_p0055.cpp)
- [45. 跳跃游戏 II](problems/0045-jump-game-ii.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0045_jump_game_ii.hpp) · [测试](../../algorithms/cpp/tests/test_p0045.cpp)
- [763. 划分字母区间](problems/0763-partition-labels.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0763_partition_labels.hpp) · [测试](../../algorithms/cpp/tests/test_p0763.cpp)

### 15. 动态规划（10题）

- [70. 爬楼梯](problems/0070-climbing-stairs.md) · EASY · [源码](../../algorithms/cpp/solutions/p0070_climbing_stairs.hpp) · [测试](../../algorithms/cpp/tests/test_p0070.cpp)
- [118. 杨辉三角](problems/0118-pascals-triangle.md) · EASY · [源码](../../algorithms/cpp/solutions/p0118_pascals_triangle.hpp) · [测试](../../algorithms/cpp/tests/test_p0118.cpp)
- [198. 打家劫舍](problems/0198-house-robber.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0198_house_robber.hpp) · [测试](../../algorithms/cpp/tests/test_p0198.cpp)
- [279. 完全平方数](problems/0279-perfect-squares.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0279_perfect_squares.hpp) · [测试](../../algorithms/cpp/tests/test_p0279.cpp)
- [322. 零钱兑换](problems/0322-coin-change.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0322_coin_change.hpp) · [测试](../../algorithms/cpp/tests/test_p0322.cpp)
- [139. 单词拆分](problems/0139-word-break.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0139_word_break.hpp) · [测试](../../algorithms/cpp/tests/test_p0139.cpp)
- [300. 最长递增子序列](problems/0300-longest-increasing-subsequence.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0300_longest_increasing_subsequence.hpp) · [测试](../../algorithms/cpp/tests/test_p0300.cpp)
- [152. 乘积最大子数组](problems/0152-maximum-product-subarray.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0152_maximum_product_subarray.hpp) · [测试](../../algorithms/cpp/tests/test_p0152.cpp)
- [416. 分割等和子集](problems/0416-partition-equal-subset-sum.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0416_partition_equal_subset_sum.hpp) · [测试](../../algorithms/cpp/tests/test_p0416.cpp)
- [32. 最长有效括号](problems/0032-longest-valid-parentheses.md) · HARD · [源码](../../algorithms/cpp/solutions/p0032_longest_valid_parentheses.hpp) · [测试](../../algorithms/cpp/tests/test_p0032.cpp)

### 16. 多维动态规划（5题）

- [62. 不同路径](problems/0062-unique-paths.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0062_unique_paths.hpp) · [测试](../../algorithms/cpp/tests/test_p0062.cpp)
- [64. 最小路径和](problems/0064-minimum-path-sum.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0064_minimum_path_sum.hpp) · [测试](../../algorithms/cpp/tests/test_p0064.cpp)
- [5. 最长回文子串](problems/0005-longest-palindromic-substring.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0005_longest_palindromic_substring.hpp) · [测试](../../algorithms/cpp/tests/test_p0005.cpp)
- [1143. 最长公共子序列](problems/1143-longest-common-subsequence.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p1143_longest_common_subsequence.hpp) · [测试](../../algorithms/cpp/tests/test_p1143.cpp)
- [72. 编辑距离](problems/0072-edit-distance.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0072_edit_distance.hpp) · [测试](../../algorithms/cpp/tests/test_p0072.cpp)

### 17. 技巧（5题）

- [136. 只出现一次的数字](problems/0136-single-number.md) · EASY · [源码](../../algorithms/cpp/solutions/p0136_single_number.hpp) · [测试](../../algorithms/cpp/tests/test_p0136.cpp)
- [169. 多数元素](problems/0169-majority-element.md) · EASY · [源码](../../algorithms/cpp/solutions/p0169_majority_element.hpp) · [测试](../../algorithms/cpp/tests/test_p0169.cpp)
- [75. 颜色分类](problems/0075-sort-colors.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0075_sort_colors.hpp) · [测试](../../algorithms/cpp/tests/test_p0075.cpp)
- [31. 下一个排列](problems/0031-next-permutation.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0031_next_permutation.hpp) · [测试](../../algorithms/cpp/tests/test_p0031.cpp)
- [287. 寻找重复数](problems/0287-find-the-duplicate-number.md) · MEDIUM · [源码](../../algorithms/cpp/solutions/p0287_find_the_duplicate_number.hpp) · [测试](../../algorithms/cpp/tests/test_p0287.cpp)

## 额外公开练习，不计入100

- [695. 岛屿的最大面积](supplementary/0695-max-area-of-island.md) · [源码](../../algorithms/cpp/supplementary/p0695_max_area_of_island.hpp) · [测试](../../algorithms/cpp/tests/test_p0695.cpp)

## 完成一题的标准

能复述输入前提与修改行为；能手推新例子；能解释状态和每次淘汰/转移为何成立；能列出最坏/平均复杂度及输出、递归栈；能闭卷实现并新增边界测试；能回答题尾至少一个追问。测试通过只是其中一部分。
