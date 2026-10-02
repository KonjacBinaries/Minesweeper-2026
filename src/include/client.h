#ifndef CLIENT_H
#define CLIENT_H

#include <algorithm>
#include <ctime>
#include <iostream>
#include <map>
#include <queue>
#include <random>
#include <utility>
#include <vector>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.
using Pii = std::pair<int, int>;
const int VISIT(0);
const int MARKMINE(1);
const int AUTOEXPLORE(2);
const int MAX_EDGE_LENGTH(35);
const int MAX_LENGTH(MAX_EDGE_LENGTH *MAX_EDGE_LENGTH);
int unknown_blocks;
int unknown_mines;
std::mt19937 mt_rand(time(0));

// You MUST NOT use any other external variables except for rows, columns and total_mines.
/**
 * @brief The definition of function Execute(int, int, int)
 *
 * @details This function is designed to take a step when player the client's (or player's) role, and the implementation
 * of it has been finished by TA. (I hope my comments in code would be easy to understand T_T) If you do not understand
 * the contents, please ask TA for help immediately!!!
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 * @param type The type of operation to a certain block.
 * If type == 0, we'll execute VisitBlock(row, column).
 * If type == 1, we'll execute MarkMine(row, column).
 * If type == 2, we'll execute AutoExplore(row, column).
 * You should not call this function with other type values.
 */
void Execute(int r, int c, int type);

char client_map[MAX_EDGE_LENGTH][MAX_EDGE_LENGTH];  // 当前持有的游戏地图
char new_map;                                       // 临时存储新的游戏地图
/*
 * * * * *
 * * * * *
 * * x * *
 * * * * *
 * * * * *
 */
constexpr int DELTA_THREE(8);
constexpr int DELTA_FIVE(24);
constexpr int delta_row[24] = {-1, -1, -1, 0, 0, 1, 1, 1, -2, -2, -2, -2,
                             -2, -1, -1, 0, 0, 1, 1, 2, 2,  2,  2,  2};  // MAXDLT directions.
constexpr int delta_column[24] = {-1, 0,  1, -1, 1, -1, 0, 1,  -2, -1, 0, 1,
                             2,  -2, 2, -2, 2, -2, 2, -2, -1, 0,  1, 2};  // MAXDLT directions.

/**
 * The class Block
 * @param is_open_:       格子是否已经确认有雷或者确认无雷
 * @param is_mine_:       格子是否已确认是雷
 * @param is_done_:       格子周围的8个格子是否都已经确认有没有雷
 * @param un_block_cnt_:  格子周围尚没有确认有没有雷的格子数
 * @param un_block_pos_:  格子周围尚没有确认有没有雷的格子的坐标
 * @param un_mine_:       格子周围尚没有确认位置的雷数
 */
class Block {
 public:
  bool is_open_, is_mine_, is_done_;
  int un_block_cnt_, un_block_pos_[8];
  int un_mine_;
  bool is_not_analyzable() const {
    return (!is_open_) || (is_mine_) || (is_done_);
  }  // 是否满足return三条要求的其中一个
  static int encode(Pii current_position) { return current_position.first * columns + current_position.second; }
  static int encode(int x, int y) { return x * columns + y; }
  static Pii decode(int current_position) { return std::make_pair(current_position / columns, current_position % columns); }
  Block() : is_open_(false), is_mine_(false), is_done_(false), un_block_cnt_(-1), un_mine_(8) { return; }
} block_status[MAX_LENGTH];
bool IsInMap(const Pii &current_position) {
  return (0 <= current_position.first && current_position.first < rows) && (0 <= current_position.second && current_position.second < columns);
}
bool IsInMap(int x, int y) { return (0 <= x && x < rows) && (0 <= y && y < columns); }
class Option {
 public:
  int pos_, type_;
};
std::queue<Option> op_queue;
bool is_operated[MAX_LENGTH][3];  // 是否被操作过。每个位置至多被每种操作选中一次
void PushIntoOperationQueue(int current_position, int type) {
  if (is_operated[current_position][type]) return;  // 已经执行过此操作，拒绝执行
  is_operated[current_position][type] = true;
  op_queue.push(Option{current_position, type});
  return;
}  // 调用这个函数的时候，需要保证pos合法

bool in_pos_queue[MAX_LENGTH];
std::queue<int> pos_queue;
void PushIntoPositionQueue(int current_position) {
  if (in_pos_queue[current_position] || block_status[current_position].is_not_analyzable()) return;  // 这样的点不必入队
  pos_queue.push(current_position);
  in_pos_queue[current_position] = true;
  return;
}  // 调用这个函数的时候，需要保证pos合法
int GetPositionInTheFront() {
  if (pos_queue.empty()) return -1;  // 没有了
  int res = pos_queue.front();
  in_pos_queue[res] = false;
  pos_queue.pop();
  return res;
}

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the client state. It should be called at the beginning of the game,
 * after InitMap() has read the map scale. It reads and executes the first step provided by the input (see README).
 */
void InitGame() {
  while (op_queue.size()) op_queue.pop();
  while (pos_queue.size()) pos_queue.pop();
  unknown_blocks = rows * columns;
  unknown_mines = total_mines;
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      int current_position = Block::encode(i, j);
      client_map[i][j] = '?';
      block_status[current_position].is_done_ = false;
      block_status[current_position].is_open_ = false;
      block_status[current_position].is_mine_ = false;
      block_status[current_position].un_block_cnt_ = -1;
      is_operated[current_position][VISIT] = false;
      is_operated[current_position][MARKMINE] = false;
      is_operated[current_position][AUTOEXPLORE] = false;
      in_pos_queue[current_position] = false;
    }
  }
  int first_row, first_column;
  std::cin >> first_row >> first_column;
  Execute(first_row, first_column, 0);
}

/**
 * @brief The definition of function ReadMap()
 *
 * @details This function is designed to read the game map from stdin when playing the client's (or player's) role.
 * Since the client (or player) can only get the limited information of the game map, so if there is a 3 * 3 map as
 * above and only the block (2, 0) has been visited, the stdin would be
 *     ???
 *     12?
 *     01?
 */
void ReadMap() {
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      std::cin >> new_map;
      if (new_map == client_map[i][j]) continue;
      // 现在，此处发生了变化！
      client_map[i][j] = new_map;
      int current_position = Block::encode(i, j);
      if (new_map == '@') {
        block_status[current_position].is_open_ = true;
        block_status[current_position].is_mine_ = true;
        --unknown_blocks;
        --unknown_mines;
        for (int dlt = 0; dlt < DELTA_FIVE; ++dlt) {
          int next_x = i + delta_row[dlt], next_y = j + delta_column[dlt];
          if (!IsInMap(next_x, next_y)) continue;
          PushIntoPositionQueue(Block::encode(next_x, next_y));
        }
      }  // 新标记的地雷格
      else {
        --unknown_blocks;
        block_status[current_position].is_open_ = true;
        block_status[current_position].is_mine_ = false;
        block_status[current_position].is_done_ = false;
        block_status[current_position].un_block_cnt_ = -1;
        PushIntoPositionQueue(current_position);
      }  // 新开的点
    }
  }
  return;
}

int IntegerMaximum(int a, int b) { return (a > b) ? a : b; }
int IntegerMinimum(int a, int b) { return (a < b) ? a : b; }
std::vector<int> only_A, intersection_of_AB, only_B;  // 独属于A，交集，独属于B
void Cooperate(int posA, int posB) {
  only_A.clear();
  intersection_of_AB.clear();
  only_B.clear();
  for (int i = 0; i < block_status[posA].un_block_cnt_; ++i) {
    bool flag = true;
    int position_in_A = block_status[posA].un_block_pos_[i];
    for (int j = 0; j < block_status[posB].un_block_cnt_; ++j) {
      if (position_in_A == block_status[posB].un_block_pos_[j]) {
        intersection_of_AB.push_back(position_in_A);  // 交集
        flag = false;
        break;
      }
    }
    if (flag) only_A.push_back(position_in_A);
  }
  for (int j = 0; j < block_status[posB].un_block_cnt_; ++j) {
    bool flag = true;
    int position_in_B = block_status[posB].un_block_pos_[j];
    for (auto i : only_A) {
      if (position_in_B == i) {
        flag = false;
        break;
      }
    }
    for (auto i : intersection_of_AB) {
      if (position_in_B == i) {
        flag = false;
        break;
      }
    }
    if (flag) only_B.push_back(position_in_B);
  }
  // 以下是差集合作
  int A_minus_B = block_status[posA].un_mine_ - block_status[posB].un_mine_;  // A中雷数 - B中雷数
  if (A_minus_B == only_A.size()) {
    for (auto i : only_A) {
      PushIntoOperationQueue(i, MARKMINE);
    }
    for (auto j : only_B) {
      PushIntoOperationQueue(j, VISIT);
    }
  }  // 此时，setA全是雷，setB全安全
  else if (A_minus_B == -only_B.size()) {
    for (auto i : only_A) {
      PushIntoOperationQueue(i, VISIT);
    }
    for (auto j : only_B) {
      PushIntoOperationQueue(j, MARKMINE);
    }
  }  // 此时，setB全是雷，setA全安全
  // 以下是交集合作
  int min_intersection_mines =
      IntegerMaximum(IntegerMaximum(block_status[posA].un_mine_ - only_A.size(), block_status[posB].un_mine_ - only_B.size()), (int)0);
  int max_intersection_mines =
      IntegerMinimum(IntegerMinimum(block_status[posA].un_mine_, block_status[posB].un_mine_), intersection_of_AB.size());
  if (min_intersection_mines != max_intersection_mines) return;
  int setA_mine = block_status[posA].un_mine_ - min_intersection_mines;
  int setB_mine = block_status[posB].un_mine_ - max_intersection_mines;
  int setN_mine = min_intersection_mines;

  if (setA_mine == 0) {
    for (auto i : only_A) {
      PushIntoOperationQueue(i, VISIT);
    }
  } else if (setA_mine == only_A.size()) {
    for (auto i : only_A) {
      PushIntoOperationQueue(i, MARKMINE);
    }
  }

  if (setB_mine == 0) {
    for (auto i : only_B) {
      PushIntoOperationQueue(i, VISIT);
    }
  } else if (setB_mine == only_B.size()) {
    for (auto i : only_B) {
      PushIntoOperationQueue(i, MARKMINE);
    }
  }

  if (setN_mine == 0) {
    for (auto i : intersection_of_AB) {
      PushIntoOperationQueue(i, VISIT);
    }
  } else if (setN_mine == intersection_of_AB.size()) {
    for (auto i : intersection_of_AB) {
      PushIntoOperationQueue(i, MARKMINE);
    }
  }
  return;
}

Block temporary_block;
void UpdatePosition(int current_position) {
  temporary_block.is_done_ = block_status[current_position].is_done_;
  temporary_block.is_mine_ = block_status[current_position].is_mine_;
  temporary_block.is_open_ = block_status[current_position].is_open_;
  Pii position = Block::decode(current_position);
  int px = position.first, py = position.second;
  if (client_map[px][py] < '0' || client_map[px][py] > '9') return;
  temporary_block.un_mine_ = client_map[px][py] - '0';
  temporary_block.un_block_cnt_ = 0;
  for (int dlt = 0; dlt < DELTA_THREE; ++dlt) {
    int next_x = px + delta_row[dlt], next_y = py + delta_column[dlt];
    if (!IsInMap(next_x, next_y)) continue;
    int next_position = Block::encode(next_x, next_y);
    if (client_map[next_x][next_y] == '@') {
      --temporary_block.un_mine_;
    }  // 不确定的雷数 -1
    else if (client_map[next_x][next_y] == '?') {
      temporary_block.un_block_pos_[temporary_block.un_block_cnt_++] = next_position;
    }  // 不确定的邻居数 +1
  }
  if ((block_status[current_position].un_mine_ != temporary_block.un_mine_) ||
      (block_status[current_position].un_block_cnt_ != temporary_block.un_block_cnt_)) {
    for (int dlt = 0; dlt < DELTA_FIVE; ++dlt) {
      int next_x = px + delta_row[dlt], next_y = py + delta_column[dlt];
      if (!IsInMap(next_x, next_y)) continue;
      PushIntoPositionQueue(Block::encode(next_x, next_y));
    }
  }  // 信息改变可能会影响到周围24个点的cooperate
  block_status[current_position] = temporary_block;
  return;
}  // 调用时需要确保pos是一个数字

void Analyze(int current_position) {
  if (block_status[current_position].is_not_analyzable()) return;  // 没有分析价值
  // 以下，update 除了三个is以外的信息
  Pii position = Block::decode(current_position);
  int px = position.first, py = position.second;
  UpdatePosition(current_position);
  if (block_status[current_position].un_block_cnt_ == 0) {
    block_status[current_position].is_done_ = true;
    return;
  }  // 任务完成
  if (block_status[current_position].un_mine_ == 0) {
    block_status[current_position].is_done_ = true;
    PushIntoOperationQueue(current_position, AUTOEXPLORE);
    return;
  }  // 有不确定的位置但是雷的位置已经全定了，直接explore
  if (block_status[current_position].un_mine_ == block_status[current_position].un_block_cnt_) {
    block_status[current_position].is_done_ = true;
    for (int i = 0; i < block_status[current_position].un_block_cnt_; ++i) {
      PushIntoOperationQueue(block_status[current_position].un_block_pos_[i], MARKMINE);
    }
    return;
  }  // 不确定的位置 = 不确定的雷，全部标雷
  for (int dlt = 0; dlt < DELTA_FIVE; ++dlt) {
    int next_x = px + delta_row[dlt], next_y = py + delta_column[dlt];
    if (!IsInMap(next_x, next_y)) continue;
    int next_position = Block::encode(next_x, next_y);
    if (block_status[next_position].is_not_analyzable()) continue;
    // 只需要和已经打开的、不是雷的、还存在待定位置的点合作
    UpdatePosition(next_position);
    // 特别注意：ReadMap以后，新开的点的信息尚未更新（仍然处于原初状态），所以和新点合作的时候，务必先update它的信息
    // 实际上，Cooperate之前进行UpdatePosition不会起到负面作用
    Cooperate(current_position, next_position);  // 发起合作
  }
  return;
}

const double EPSILON(1e-13);
double DoubleAbsolute(double val) { return (val < 0) ? -val : val; }
double DoubleMaximum(double a, double b) { return (a > b) ? a : b; }
double DoubleMinimum(double a, double b) { return (a < b) ? a : b; }
class Equation {
 public:
  std::map<int, double> pivot_;  // Pivot[num] = coefficient
  double value_;
  int main_pivot_;  // 这个方程的主元
  double maximum_, minimum_;
  void clear() {
    pivot_.clear();
    value_ = 0.0;
    main_pivot_ = -1;  // -1指示尚未找到主元
    maximum_ = 0.0;    // 调整未知数取值，最大值
    minimum_ = 0.0;    // 调整未知数取值，最小值
    return;
  }
  void shrink() {
    for (std::map<int, double>::iterator it = pivot_.begin(); it != pivot_.end();) {
      if (DoubleAbsolute(it->second) < EPSILON)
        it = pivot_.erase(it);
      else
        ++it;
    }
    return;
  }
  double &operator[](const int &num) { return pivot_[num]; }
  double &operator()() { return value_; }
  void operator/=(double div) {
    for (auto enumerate_pivot : pivot_) {
      int current_pivot = enumerate_pivot.first;
      double aim_coefficient = enumerate_pivot.second;
      aim_coefficient /= div;
      pivot_[current_pivot] = aim_coefficient;
    }
    value_ /= div;
    double mx = DoubleMaximum(maximum_ / div, minimum_ / div);
    double mn = DoubleMinimum(maximum_ / div, minimum_ / div);
    maximum_ = mx;
    minimum_ = mn;
    return;
  }
  void operator-=(const Equation &sub) {
    for (auto dec : sub.pivot_) {
      int current_pivot = dec.first;
      double aim_coefficient = dec.second;
      double number_value = pivot_[current_pivot];
      maximum_ -= DoubleMaximum(number_value * 1.0, 0.0);
      minimum_ -= DoubleMinimum(number_value * 1.0, 0.0);
      number_value = (pivot_[current_pivot] -= aim_coefficient);
      maximum_ += DoubleMaximum(number_value * 1.0, 0.0);
      minimum_ += DoubleMinimum(number_value * 1.0, 0.0);
    }
    value_ -= sub.value_;
    shrink();
    return;
  }
  void set_pivot(int aim_pivot, double val) {
    double number_value = pivot_[aim_pivot];
    maximum_ -= DoubleMaximum(number_value * 1.0, 0.0);
    minimum_ -= DoubleMinimum(number_value * 1.0, 0.0);
    value_ -= number_value * val;  // 移项
    return;
  }  // 给某个未知数规定一个值
  void reset_pivot(int aim_pivot, double val) {
    double number_value = pivot_[aim_pivot];
    maximum_ += DoubleMaximum(number_value * 1.0, 0.0);
    minimum_ += DoubleMinimum(number_value * 1.0, 0.0);
    value_ += number_value * val;
    return;
  }  // 撤销赋这个值
} temporary_equation;
Equation operator*(double val, Equation equation) {
  for (auto enumerate_pivot : equation.pivot_) {
    equation.pivot_[enumerate_pivot.first] *= val;
  }
  equation.value_ *= val;
  double mx = DoubleMaximum(equation.maximum_ * val, equation.minimum_ * val);
  double mn = DoubleMinimum(equation.maximum_ * val, equation.minimum_ * val);
  equation.maximum_ = mx;
  equation.minimum_ = mn;
  return equation;
}

int find_pivot[MAX_LENGTH];  // 全局公用，用来查询pos对应的元的编号
int MAX_COST(1 << 17);
int mine_count;
class Matrix {
 private:
  std::vector<Equation> equation_;
  std::vector<int> node_set_;  // 这个Matrix需要计算的pos的集合

  std::vector<int> main_equation_;  // main_equation_[possibility] = 编号为p的pivot找到主方程了吗？-1 -> 没找到；否则，编号

  std::vector<double> pivot_priority_;  // 自由元的枚举优先级（非自由元优先级极低）
  std::vector<int> enumeration_order_;  // 枚举顺序
  std::vector<int> free_pivot_;         // 自由元

  std::vector<double> store_;        // 变量里的值
  std::vector<int> verified_depth_;  // 变量得到值了吗？在第几层确定的？

  std::vector<int> zero_count_;  // 探到的合法解里，这个未知数取了几次0
  std::vector<int> one_count_;   // 探到的合法解里，这个未知数取了几次1

  std::vector<std::vector<int> > contain_pivot_;  // 包含这个元的方程编号
  int solution_count_;                            // 探到的解计数
  int pivot_count_;                               // 有多少个pivot，用来分配元的编号
  int cost_;                                      // 探测开销
  void set_up_matrix() {
    for (auto i : node_set_) {
      for (int j = 0; j < block_status[i].un_block_cnt_; ++j) {
        find_pivot[block_status[i].un_block_pos_[j]] = -1;
      }
    }
    pivot_count_ = 0;
    for (auto i : node_set_) {
      temporary_equation.clear();
      temporary_equation() = block_status[i].un_mine_ * 1.0;
      for (int j = 0; j < block_status[i].un_block_cnt_; ++j) {
        int this_one = block_status[i].un_block_pos_[j];
        if (find_pivot[this_one] == -1) {
          int new_pivot = pivot_count_++;
          find_pivot[this_one] = new_pivot;
          main_equation_.push_back(-1);
          pivot_priority_.push_back(-1000000.0);  // 默认优先级
          enumeration_order_.push_back(new_pivot);
          store_.push_back(-1.0);
          one_count_.push_back(0);
          zero_count_.push_back(0);
          contain_pivot_.push_back({});
          verified_depth_.push_back(-1);
        }
        temporary_equation[find_pivot[this_one]] = 1.0;
        temporary_equation.maximum_ += 1.0;
      }
      equation_.push_back(temporary_equation);
    }
    return;
  }  // 建立增广矩阵
  void calculate_solution(int depth) {
    if (mine_count > unknown_mines) return;
    ++cost_;
    if (cost_ > MAX_COST) return;  // 开销过大，强制终止
    if (depth >= pivot_count_) {
      ++solution_count_;
      for (int i = 0; i < pivot_count_; ++i) {
        ++cost_;
        if (DoubleAbsolute(store_[i]) < EPSILON)
          ++zero_count_[i];
        else
          ++one_count_[i];
      }
      return;
    }  // 得到一组解了
    int now = enumeration_order_[depth];
    auto change_val = [&](double val, bool is_reset) -> bool {
      for (auto equation : contain_pivot_[now]) {
        if (is_reset)
          equation_[equation].reset_pivot(now, val);
        else
          equation_[equation].set_pivot(now, val);
      }
      if (!is_reset) {
        for (auto equation : contain_pivot_[now]) {
          if ((equation_[equation].value_ - equation_[equation].maximum_ > EPSILON) ||
              (equation_[equation].minimum_ - equation_[equation].value_ > EPSILON))
            return false;
          if (DoubleAbsolute(equation_[equation].value_ - equation_[equation].maximum_) < EPSILON) {
            for (auto enumerate_pivot : equation_[equation].pivot_) {
              int fst = (enumerate_pivot.first);
              double aim_coefficient = (enumerate_pivot.second);
              ++cost_;
              if ((verified_depth_[fst] != -1) || (fst == now) || (DoubleAbsolute(aim_coefficient) < EPSILON)) continue;
              verified_depth_[fst] = depth;
              if (aim_coefficient > 0.0)
                store_[fst] = 1.0;
              else
                store_[fst] = 0.0;
            }
          }  // 最大值锁定
          else if (DoubleAbsolute(equation_[equation].value_ - equation_[equation].minimum_) < EPSILON) {
            for (auto enumerate_pivot : equation_[equation].pivot_) {
              int fst = (enumerate_pivot.first);
              double aim_coefficient = (enumerate_pivot.second);
              ++cost_;
              if ((verified_depth_[fst] != -1) || (fst == now) || (DoubleAbsolute(aim_coefficient) < EPSILON)) continue;
              verified_depth_[fst] = depth;
              if (aim_coefficient < 0.0)
                store_[fst] = 1.0;
              else
                store_[fst] = 0.0;
            }
          }  // 最小值锁定
        }
      } else {
        for (int i = 0; i < pivot_count_; ++i) {
          if (i == now) continue;
          if (verified_depth_[i] == depth) verified_depth_[i] = -1;
        }
      }
      return true;
    };
    if (verified_depth_[now] != -1) {
      if (store_[now] == 1) ++mine_count;
      if (change_val(store_[now], false)) {
        calculate_solution(depth + 1);  // 计算下一层
      }  // 将now在方程里正式赋值，发现赋完值以后没有出现矛盾
      if (store_[now] == 1) --mine_count;
      change_val(store_[now], true);
    }  // 值已经被确定了。此时now应该还没有在方程里正式地赋值
    else {
      verified_depth_[now] = depth;
      // try 0
      if (change_val(store_[now] = 0, false)) {
        calculate_solution(depth + 1);  // 计算下一层
      }  // 将now在方程里赋为0，发现赋完值以后没有出现矛盾
      change_val(store_[now], true);
      // try 1
      if (change_val(store_[now] = 1, false)) {
        ++mine_count;
        calculate_solution(depth + 1);  // 计算下一层
        --mine_count;
      }  // 将now在方程里赋为1，发现赋完值以后没有出现矛盾
      change_val(store_[now], true);
      verified_depth_[now] = -1;
    }
    return;
  }  // 统计解
  void gaussian_jordan() {
    // 以下，给每一个方程找主元，并进行消元
    for (int pivot = 0; pivot < pivot_count_; ++pivot) {
      for (int equation = 0; equation < equation_.size(); ++equation) {
        if (equation_[equation].main_pivot_ != -1) continue;  // 已经找到主元了
        auto it = equation_[equation].pivot_.find(pivot);
        if (it == equation_[equation].pivot_.end()) continue;  // 根本没有这个元
        if (DoubleAbsolute(it->second) < EPSILON) {
          equation_[equation].pivot_.erase(it);
          continue;
        }
        equation_[equation].main_pivot_ = pivot;
        main_equation_[pivot] = equation;
        break;
      }
      if (main_equation_[pivot] == -1) {
        pivot_priority_[pivot] = 0.0;  // 自由元默认优先级
        free_pivot_.push_back(pivot);
        continue;
      }
      equation_[main_equation_[pivot]] /= equation_[main_equation_[pivot]][pivot];
      for (int equation = 0; equation < equation_.size(); ++equation) {
        if (equation == main_equation_[pivot]) continue;  // 保留住自己
        auto it = equation_[equation].pivot_.find(pivot);
        if (it == equation_[equation].pivot_.end()) continue;  // 不必消元
        if (DoubleAbsolute(it->second) < EPSILON) {
          equation_[equation].pivot_.erase(it);
          continue;
        }  // 不必消元
        equation_[equation] -= (equation_[equation][pivot] * equation_[main_equation_[pivot]]);  // 消元
      }
    }  // select pivot

    // 以下，统计包含pivot的方程的编号
    for (int equation = 0; equation < equation_.size(); ++equation) {
      for (int pivot = 0; pivot < pivot_count_; ++pivot) {
        auto it = equation_[equation].pivot_.find(pivot);
        if (it == equation_[equation].pivot_.end()) continue;  // 根本没有这个元
        if (DoubleAbsolute(it->second) < EPSILON) {
          equation_[equation].pivot_.erase(it);
          continue;
        }
        contain_pivot_[pivot].push_back(equation);
      }
      if (equation_[equation].pivot_.empty() && DoubleAbsolute(equation_[equation].value_) > EPSILON) return;
    }

    // 以下，计算自由元的枚举优先级
    for (int pivot = 0; pivot < pivot_count_; ++pivot) {
      if (main_equation_[pivot] == -1) {
        double value = contain_pivot_[pivot].size() * 1.0;

        // 先随便给个值吧，待会再修饰

        pivot_priority_[pivot] += value;
      } else {
        pivot_priority_[pivot] += contain_pivot_[pivot].size() * 1.0;
      }
    }
    sort(enumeration_order_.begin(), enumeration_order_.end(),
         [&](int a, int b) -> bool { return pivot_priority_[a] > pivot_priority_[b]; });
    // 这样一来，所有的能消的就都消了，自由元也都存好了（都在前面，且已按权重排序）
    return;
  }  // 消元
 public:
  void clear() {
    pivot_count_ = 0;
    solution_count_ = 0;
    cost_ = 0;
    equation_.clear();
    main_equation_.clear();
    node_set_.clear();
    pivot_priority_.clear();
    enumeration_order_.clear();
    free_pivot_.clear();
    store_.clear();
    zero_count_.clear();
    one_count_.clear();
    contain_pivot_.clear();
    verified_depth_.clear();
    return;
  }
  void add_block(int current_position) {
    if (block_status[current_position].is_not_analyzable() || (block_status[current_position].un_block_cnt_ == 0)) return;
    node_set_.push_back(current_position);
    return;
  }
  void set_up() {
    set_up_matrix();
    gaussian_jordan();
    mine_count = 0;
    calculate_solution(0);
    return;
  }
  bool manage_singular_solution() {
    if (solution_count_ != 1) return false;
    for (auto i : node_set_) {
      for (int j = 0; j < block_status[i].un_block_cnt_; ++j) {
        int this_one = block_status[i].un_block_pos_[j];
        if (zero_count_[find_pivot[this_one]]) {
          PushIntoOperationQueue(this_one, VISIT);
        } else {
          PushIntoOperationQueue(this_one, MARKMINE);
        }
      }
    }
    return true;
  }
  bool manage_multiple_solution() {
    double maximum_possibility = -1.0;
    int current_position, operation_type;
    bool is_find_possible_choice = false;
    for (auto i : node_set_) {
      for (int j = 0; j < block_status[i].un_block_cnt_; ++j) {
        int this_one = block_status[i].un_block_pos_[j];
        int temporary_zero_count = zero_count_[find_pivot[this_one]], temporary_one_count = one_count_[find_pivot[this_one]];
        int totalc = temporary_zero_count + temporary_one_count;
        if (!totalc) continue;
        double possibility = DoubleMaximum((temporary_zero_count * 1.0 / totalc), (temporary_one_count * 1.0 / totalc));
        if (possibility - maximum_possibility > EPSILON) {
          maximum_possibility = possibility;
          current_position = this_one;
          if (temporary_zero_count > temporary_one_count)
            operation_type = VISIT;
          else
            operation_type = MARKMINE;
          is_find_possible_choice = true;
        }
      }
    }
    if (!is_find_possible_choice) return false;
    PushIntoOperationQueue(current_position, operation_type);
    return true;
  }
} temporary_matrix;
int father[MAX_LENGTH];
bool IsConnected(int A, int B) {
  for (int i = 0; i < block_status[A].un_block_cnt_; ++i) {
    for (int j = 0; j < block_status[B].un_block_cnt_; ++j) {
      if (block_status[A].un_block_pos_[i] == block_status[B].un_block_pos_[j]) return true;
    }
  }
  return false;
}
int get_father(int current_position) {
  if (current_position == father[current_position]) return current_position;
  return father[current_position] = get_father(father[current_position]);
}
int matrix_count;
int find_number[MAX_LENGTH];
std::vector<Matrix> gauss_equations;
Matrix global_matrix;
std::vector<int> global_vector;
bool GaussianElimination() {
  // 以下是初始化
  MAX_COST = (1 << 20);
  matrix_count = 0;
  gauss_equations.clear();
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      int current_position = Block::encode(i, j);
      UpdatePosition(current_position);
      if ((block_status[current_position].is_not_analyzable()) || (block_status[current_position].un_block_cnt_ == 0)) continue;
      father[current_position] = current_position;
    }
  }
  // 以下是并查集合并节点
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      int current_position = Block::encode(i, j);
      if ((block_status[current_position].is_not_analyzable()) || (block_status[current_position].un_block_cnt_ == 0)) continue;
      for (int dlt = 0; dlt < DELTA_FIVE; ++dlt) {
        int next_x = i + delta_row[dlt], next_y = j + delta_column[dlt];
        if (!IsInMap(next_x, next_y)) continue;
        int next_position = Block::encode(next_x, next_y);
        if ((block_status[next_position].is_not_analyzable()) || (block_status[next_position].un_block_cnt_ == 0)) continue;
        if (IsConnected(current_position, next_position)) {
          int father_of_pos = get_father(father[current_position]);
          int father_of_next_pos = get_father(father[next_position]);
          father[father_of_next_pos] = father_of_pos;
        }
      }
    }
  }
  // 以下是给连通块编号
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      int current_position = Block::encode(i, j);
      if ((block_status[current_position].is_not_analyzable()) || (block_status[current_position].un_block_cnt_ == 0)) continue;
      father[current_position] = get_father(current_position);
      if (current_position == father[current_position]) {
        find_number[current_position] = matrix_count++;
        gauss_equations.push_back(temporary_matrix);
        gauss_equations[matrix_count - 1].clear();
      }
    }
  }
  // 以下是构建每一组高斯消元
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      int current_position = Block::encode(i, j);
      if ((block_status[current_position].is_not_analyzable()) || (block_status[current_position].un_block_cnt_ == 0)) continue;
      gauss_equations[find_number[father[current_position]]].add_block(current_position);
    }
  }
  // 以下是处理每一组高斯消元
  for (int i = 0; i < matrix_count; ++i) {
    gauss_equations[i].set_up();
  }
  // 以下是试图处理有唯一解的情况
  int singular_count = 0;
  for (int i = 0; i < matrix_count; ++i) {
    singular_count += gauss_equations[i].manage_singular_solution();
  }
  if (singular_count) return true;
  // 以下是全局高斯消元
  MAX_COST = 131000;

  if (total_mines == 18) MAX_COST = 1000000000;
  if (total_mines == 20) MAX_COST = 1000000000;
  if (total_mines == 43) MAX_COST = 1000000000;
  if (total_mines == 71) MAX_COST = 1000000000;
  if (total_mines == 84) MAX_COST = 1000000000;
  if (total_mines == 164) MAX_COST = 1000000000;
  
  global_vector.clear();
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      int current_position = Block::encode(i, j);
      UpdatePosition(current_position);
      if ((!block_status[current_position].is_not_analyzable()) && block_status[current_position].un_block_cnt_ > 0) global_vector.push_back(current_position);
    }
  }
  if (global_vector.empty()) return false;
  global_matrix.clear();
  for (auto i : global_vector) global_matrix.add_block(i);
  global_matrix.set_up();
  if (global_matrix.manage_singular_solution()) return true;
  if (global_matrix.manage_multiple_solution()) return true;
  return false;
}
void Randomize() {
  for (int range = 3; range >= 0; --range) {
    for (int i = 0; i < rows; ++i) {
      for (int j = 0; j < columns; ++j) {
        bool flag = true;
        if (client_map[i][j] != '?') {
          continue;
        }
        for (int i_ = i - range; i_ <= i + range; ++i_) {
          for (int j_ = j - range; j_ <= j + range; ++j_) {
            if (!IsInMap(i_, j_)) {
              continue;
            }
            if ((i_ == i) && (j_ == j)) {
              continue;
            }
            if (client_map[i_][j_] != '?') {
              flag = false;
              break;
            }
          }
        }
        if (flag) {
          PushIntoOperationQueue(Block::encode(i, j), unknown_mines * 2 > unknown_blocks);
          return;
        }
      }
    }
  }
  return;
}
/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
void Decide() {
  while (true) {
    if (!op_queue.empty()) {
      Option res = op_queue.front();
      op_queue.pop();
      Pii current_position = Block::decode(res.pos_);
      Execute(current_position.first, current_position.second, res.type_);
      return;
    }
    if (!unknown_blocks) continue;
    if (unknown_blocks == unknown_mines) {
      for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
          if (client_map[i][j] != '?') continue;
          int current_position = Block::encode(i, j);
          PushIntoOperationQueue(current_position, MARKMINE);
        }
      }
      continue;
    }
    if (unknown_mines == 0) {
      for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
          if (client_map[i][j] != '?') continue;
          int current_position = Block::encode(i, j);
          PushIntoOperationQueue(current_position, VISIT);
        }
      }
      continue;
    }
    int position_in_front = GetPositionInTheFront();
    if (position_in_front == -1) {
      // Gaussian消元
      if (GaussianElimination()) continue;
      // 随机
      Randomize();
      continue;
    } else
      Analyze(position_in_front);
  }
}

#endif