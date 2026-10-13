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
#include <algorithm>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.
using Pii = std::pair<int, int>;
constexpr int VISIT(0);
constexpr int MARKMINE(1);
constexpr int AUTOEXPLORE(2);
constexpr int MAX_EDGE_LENGTH(35);
constexpr int MAX_LENGTH(MAX_EDGE_LENGTH * MAX_EDGE_LENGTH);
int unknown_blocks;
int unknown_mines;

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
char temporary_char;                                // 临时存储新的游戏地图
constexpr int DELTA_THREE(8);
constexpr int DELTA_FIVE(24);
constexpr int delta_row[24] = {-1, -1, -1, 0, 0, 1, 1, 1, -2, -2, -2, -2,
                             -2, -1, -1, 0, 0, 1, 1, 2, 2,  2,  2,  2};  
constexpr int delta_column[24] = {-1, 0,  1, -1, 1, -1, 0, 1,  -2, -1, 0, 1,
                             2,  -2, 2, -2, 2, -2, 2, -2, -1, 0,  1, 2};  

/**
 * The class Block
 * @param is_confirmed_status_:          格子是否已经确认有雷或者确认无雷
 * @param is_confirmed_mine_:            格子是否已确认是雷
 * @param is_all_confirmed_:             格子周围的8个格子是否都已经确认有没有雷
 * @param unknown_block_count_:          格子周围尚没有确认有没有雷的格子数
 * @param unknown_block_position_:       格子周围尚没有确认有没有雷的格子的坐标
 * @param unknown_mine_count_:           格子周围尚没有确认位置的雷数
 */
class Block {
 public:
  bool is_confirmed_status_, is_confirmed_mine_, is_all_confirmed_;
  int unknown_block_count_, unknown_block_position_[8];
  int unknown_mine_count_;
  bool is_not_worthy_analyzing() const {
    return (!is_confirmed_status_) || is_confirmed_mine_ || is_all_confirmed_;
  } // 是否没有任何分析价值
  static int encode(Pii current_position) { 
    return current_position.first * columns + current_position.second; 
  } // 对Pii类型的坐标进行编码
  static int encode(int x, int y) { 
    return x * columns + y;
  } // 对形如(x, y)的坐标进行编码
  static Pii decode(int current_position) {
      return std::make_pair(current_position / columns, current_position % columns); 
  } // 对编码进行解码
  Block() : 
    is_confirmed_status_(false), 
    is_confirmed_mine_(false), 
    is_all_confirmed_(false), 
    unknown_block_count_(-1), 
    unknown_mine_count_(8) {} // 构造函数
} block_status[MAX_LENGTH], temporary_block;

bool IsInMap(const Pii &current_position) {
  return (0 <= current_position.first && current_position.first < rows) && (0 <= current_position.second && current_position.second < columns);
} // 判断位置是否在地图内
bool IsInMap(int x, int y) {
    return (0 <= x && x < rows) && (0 <= y && y < columns); 
 } // 判断位置是否在地图内

/**
 * The class Option
 * @param position_:                     操作的位置
 * @param type_:                         操作的类型
 */
class Option {
 public:
  int position_, type_;
};
std::queue<Option> operation_queue;
bool is_operated[MAX_LENGTH][3];  // 是否被操作过。每个位置至多被每种操作选中一次
void PushIntoOperationQueue(int current_position, int type) {
  if (is_operated[current_position][type]) return;  // 已经执行过此操作，拒绝执行
  is_operated[current_position][type] = true;
  operation_queue.push(Option{current_position, type});
  return;
} // 新增操作（注意，调用这个函数的时候，需要保证current_position合法）

bool in_position_queue[MAX_LENGTH]; // 是否在队列里
std::queue<int> position_queue;
void PushIntoPositionQueue(int current_position) {
  if (in_position_queue[current_position] || 
      block_status[current_position].is_not_worthy_analyzing()) {
    return;  // 这样的点不必入队
  }
  position_queue.push(current_position);
  in_position_queue[current_position] = true;
  return;
} // 新增要分析的点（调用这个函数的时候，需要保证current_position合法）
int GetPositionInTheFront() {
  if (position_queue.empty()) return -1;  // 没有了
  int get_position = position_queue.front();
  in_position_queue[get_position] = false;
  position_queue.pop();
  return get_position;
} // 获得下一个要分析的点

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the client state. It should be called at the beginning of the game,
 * after InitMap() has read the map scale. It reads and executes the first step provided by the input (see README).
 */
void InitGame() {
  while (operation_queue.size()) operation_queue.pop();
  while (position_queue.size()) position_queue.pop();
  unknown_blocks = rows * columns;
  unknown_mines = total_mines;
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      int current_position = Block::encode(i, j);
      client_map[i][j] = '?';
      block_status[current_position].is_all_confirmed_ = false;
      block_status[current_position].is_confirmed_status_ = false;
      block_status[current_position].is_confirmed_mine_ = false;
      block_status[current_position].unknown_block_count_ = -1;
      is_operated[current_position][VISIT] = false;
      is_operated[current_position][MARKMINE] = false;
      is_operated[current_position][AUTOEXPLORE] = false;
      in_position_queue[current_position] = false;
    }
  }
  int first_row, first_column;
  std::cin >> first_row >> first_column;
  Execute(first_row, first_column, 0);
} // 初始化游戏

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
      std::cin >> temporary_char;
      if (temporary_char == client_map[i][j]) continue;
      // 现在，此处发生了变化！
      client_map[i][j] = temporary_char;
      int current_position = Block::encode(i, j);
      if (temporary_char == '@') {
        block_status[current_position].is_confirmed_status_ = true;
        block_status[current_position].is_confirmed_mine_ = true;
        --unknown_blocks;
        --unknown_mines;
        for (int enumerate_delta = 0; enumerate_delta < DELTA_FIVE; ++enumerate_delta) {
          int next_x = i + delta_row[enumerate_delta], next_y = j + delta_column[enumerate_delta];
          if (!IsInMap(next_x, next_y)) continue;
          PushIntoPositionQueue(Block::encode(next_x, next_y));
        }
      } // 新标记的地雷格
      else {
        --unknown_blocks;
        block_status[current_position].is_confirmed_status_ = true;
        block_status[current_position].is_confirmed_mine_ = false;
        block_status[current_position].is_all_confirmed_ = false;
        block_status[current_position].unknown_block_count_ = -1;
        PushIntoPositionQueue(current_position);
      } // 新开的点
    }
  }
  return;
} // 读入地图，并处理变更

void UpdatePosition(int current_position) {
  if (block_status[current_position].is_not_worthy_analyzing()) return;
  temporary_block.is_all_confirmed_ = block_status[current_position].is_all_confirmed_;
  temporary_block.is_confirmed_mine_ = block_status[current_position].is_confirmed_mine_;
  temporary_block.is_confirmed_status_ = block_status[current_position].is_confirmed_status_;
  Pii position = Block::decode(current_position);
  int position_x = position.first, position_y = position.second;
  if (client_map[position_x][position_y] < '0' || client_map[position_x][position_y] > '9') return;
  temporary_block.unknown_mine_count_ = client_map[position_x][position_y] - '0';
  temporary_block.unknown_block_count_ = 0;
  for (int enumerate_delta = 0; enumerate_delta < DELTA_THREE; ++enumerate_delta) {
    int next_x = position_x + delta_row[enumerate_delta];
    int next_y = position_y + delta_column[enumerate_delta];
    if (!IsInMap(next_x, next_y)) continue;
    int next_position = Block::encode(next_x, next_y);
    if (client_map[next_x][next_y] == '@') {
      --temporary_block.unknown_mine_count_;
    } // 不确定的雷数 -1
    else if (client_map[next_x][next_y] == '?') {
      temporary_block.unknown_block_position_[temporary_block.unknown_block_count_++] = next_position;
    } // 不确定的邻居数 +1
  }
  if ((block_status[current_position].unknown_mine_count_ != temporary_block.unknown_mine_count_) ||
      (block_status[current_position].unknown_block_count_ != temporary_block.unknown_block_count_)) {
    for (int enumerate_delta = 0; enumerate_delta < DELTA_FIVE; ++enumerate_delta) {
      int next_x = position_x + delta_row[enumerate_delta];
      int next_y = position_y + delta_column[enumerate_delta];
      if (!IsInMap(next_x, next_y)) continue;
      PushIntoPositionQueue(Block::encode(next_x, next_y));
    }
  } // 信息改变可能会影响到周围24个点的小型高斯消元
  block_status[current_position] = temporary_block;
  return;
} // 更新方格信息（调用时需要确保current_position所在位置是一个数字）

constexpr double EPSILON(1e-13);

/**
 * The class Equation
 * @param variable_:                     存储变量的系数
 * @param value_:                        存储方程右侧的值
 * @param main_variable_:                该方程的主元
 * @param maximum_:                      通过调整变量，方程左侧的最大值
 * @param minimum_:                      通过调整变量，方程左侧的最小值
 */
class Equation {
 public:
  std::map<int, double> variable_;
  double value_;
  int main_variable_;
  double maximum_, minimum_;
  void clear() {
    variable_.clear();
    value_ = 0.0;
    main_variable_ = -1;  // -1指示尚未找到主元
    maximum_ = 0.0;    // 调整未知数取值，最大值
    minimum_ = 0.0;    // 调整未知数取值，最小值
    return;
  } // 清空方程
  void shrink_equation() {
    for (std::map<int, double>::iterator it = variable_.begin(); it != variable_.end();) {
      if (std::abs(it->second) < EPSILON)
        it = variable_.erase(it);
      else
        ++it;
    }
    return;
  } // 清除无意义的变量
  double &operator[](const int &number) {
      return variable_[number];
  } // 引用系数
  double &operator()() {
    return value_;
  } // 引用值
  void operator/=(double divisor) {
    for (auto enumerate_variable : variable_) {
      int current_variable = enumerate_variable.first;
      double aim_coefficient = enumerate_variable.second;
      aim_coefficient /= divisor;
      variable_[current_variable] = aim_coefficient;
    }
    value_ /= divisor;
    double maximum = std::max(maximum_ / divisor, minimum_ / divisor);
    double minimum = std::min(maximum_ / divisor, minimum_ / divisor);
    maximum_ = maximum;
    minimum_ = minimum;
    return;
  } // 除以常数
  void operator-=(const Equation &subtraction) {
    for (auto affected_variable : subtraction.variable_) {
      int current_variable = affected_variable.first;
      double aim_coefficient = affected_variable.second;
      double number_value = variable_[current_variable];
      maximum_ -= std::max(number_value * 1.0, 0.0);
      minimum_ -= std::min(number_value * 1.0, 0.0);
      number_value = (variable_[current_variable] -= aim_coefficient);
      maximum_ += std::max(number_value * 1.0, 0.0);
      minimum_ += std::min(number_value * 1.0, 0.0);
    }
    value_ -= subtraction.value_;
    shrink_equation();
    return;
  } // 减去方程
  void set_variable(int aim_variable, double value) {
    double number_value = variable_[aim_variable];
    maximum_ -= std::max(number_value * 1.0, 0.0);
    minimum_ -= std::min(number_value * 1.0, 0.0);
    value_ -= number_value * value;  // 移项
    return;
  } // 给某个未知数赋值
  void reset_variable(int aim_variable, double value) {
    double number_value = variable_[aim_variable];
    maximum_ += std::max(number_value * 1.0, 0.0);
    minimum_ += std::min(number_value * 1.0, 0.0);
    value_ += number_value * value;
    return;
  } // 撤销赋值，需要给定先前赋的值
} temporary_equation;
Equation operator*(double value, Equation equation) {
  for (auto enumerate_variable : equation.variable_) {
    equation.variable_[enumerate_variable.first] *= value;
  }
  equation.value_ *= value;
  double maximum = std::max(equation.maximum_ * value, equation.minimum_ * value);
  double minimum = std::min(equation.maximum_ * value, equation.minimum_ * value);
  equation.maximum_ = maximum;
  equation.minimum_ = minimum;
  return equation;
} // 乘常数（我希望常数作为左操作数）

int find_variable[MAX_LENGTH];  // 全局公用，用来查询pos对应的元的编号
int mine_count; // 计数，记录已经钦定了多少雷

/**
 * The class Matrix
 * @param equation_:                     增广矩阵所描述的方程
 * @param node_set_:                     建立增广矩阵所涉及的数字方格
 * @param main_equation_:                变量所对应的主方程
 * @param variable_priority_:            变量枚举优先级（自由元优先级远高于非自由元）
 * @param enumeration_order_:            变量枚举顺序
 * @param free_variable_:                自由元
 * @param store_:                        暂时为变量钦定的值
 * @param verified_depth_:               变量被钦定值时的搜索深度
 * @param zero_count_:                   搜索到的合法解中，该变量取了几次0
 * @param one_count_:                    搜索到的合法解中，该变量取了几次1
 * @param in_which_equations_:           该变量出现在了哪些方程中
 * @param solution_count_:               搜索到的解的数量
 * @param variable_count_:               未知数的数量
 */
class Matrix {
 private:
  std::vector<Equation> equation_;
  std::vector<int> node_set_;
  std::vector<int> main_equation_;
  std::vector<double> variable_priority_;
  std::vector<int> enumeration_order_;
  std::vector<int> free_variable_;
  std::vector<double> store_; 
  std::vector<int> verified_depth_;
  std::vector<int> zero_count_; 
  std::vector<int> one_count_; 
  std::vector<std::vector<int> > in_which_equations_;
  int solution_count_;
  int variable_count_;
  void set_up_matrix() {
    for (auto i : node_set_) {
      for (int j = 0; j < block_status[i].unknown_block_count_; ++j) {
        find_variable[block_status[i].unknown_block_position_[j]] = -1;
      }
    } // 初始化
    variable_count_ = 0;
    for (auto i : node_set_) {
      temporary_equation.clear();
      temporary_equation() = block_status[i].unknown_mine_count_ * 1.0;
      for (int j = 0; j < block_status[i].unknown_block_count_; ++j) {
        int this_one = block_status[i].unknown_block_position_[j];
        if (find_variable[this_one] == -1) {
          int new_variable = variable_count_++;
          find_variable[this_one] = new_variable;
          main_equation_.push_back(-1);
          variable_priority_.push_back(-1000000.0);  // 这是所有变量的默认枚举优先级
          enumeration_order_.push_back(new_variable);
          store_.push_back(-1.0);
          one_count_.push_back(0);
          zero_count_.push_back(0);
          in_which_equations_.push_back({});
          verified_depth_.push_back(-1);
        }
        temporary_equation[find_variable[this_one]] = 1.0;
        temporary_equation.maximum_ += 1.0;
      }
      equation_.push_back(temporary_equation);
    }
    return;
  } // 建立增广矩阵
  void calculate_solution(int depth) {
    if (mine_count > unknown_mines) return; // 钦定的雷已经多于剩余雷数，剪枝
    if (depth >= variable_count_) {
      ++solution_count_;
      for (int i = 0; i < variable_count_; ++i) {
        if (std::abs(store_[i]) < EPSILON)
          ++zero_count_[i];
        else
          ++one_count_[i];
      }
      return;
    } // 搜到了一组合法解
    int current_variable = enumeration_order_[depth];
    auto change_variable_value = [&](double value, bool is_reset) -> bool {
      for (auto equation : in_which_equations_[current_variable]) {
        if (is_reset)
          equation_[equation].reset_variable(current_variable, value);
        else
          equation_[equation].set_variable(current_variable, value);
      }

      if (!is_reset) {
        for (auto equation : in_which_equations_[current_variable]) {
          if ((equation_[equation].value_ - equation_[equation].maximum_ > EPSILON) ||
              (equation_[equation].minimum_ - equation_[equation].value_ > EPSILON))
            return false;
          if (std::abs(equation_[equation].value_ - equation_[equation].maximum_) < EPSILON) {
            for (auto enumerate_variable : equation_[equation].variable_) {
              int aim_variable = (enumerate_variable.first);
              double aim_coefficient = (enumerate_variable.second);
              if ((verified_depth_[aim_variable] != -1) || 
                  (aim_variable == current_variable) || 
                  (std::abs(aim_coefficient) < EPSILON)) continue;
              verified_depth_[aim_variable] = depth;
              if (aim_coefficient > 0.0)
                store_[aim_variable] = 1.0;
              else
                store_[aim_variable] = 0.0;
            }
          } // 最大值锁定
          else if (std::abs(equation_[equation].value_ - equation_[equation].minimum_) < EPSILON) {
            for (auto enumerate_variable : equation_[equation].variable_) {
              int aim_variable = (enumerate_variable.first);
              double aim_coefficient = (enumerate_variable.second);
              if ((verified_depth_[aim_variable] != -1) || 
                  (aim_variable == current_variable) || 
                  (std::abs(aim_coefficient) < EPSILON)) continue;
              verified_depth_[aim_variable] = depth;
              if (aim_coefficient < 0.0)
                store_[aim_variable] = 1.0;
              else
                store_[aim_variable] = 0.0;
            }
          } // 最小值锁定
        }
      }
      else {
        for (int i = 0; i < variable_count_; ++i) {
          if (i == current_variable) continue;
          if (verified_depth_[i] == depth) verified_depth_[i] = -1;
        }
      }
      return true;
    }; // 进行赋值与撤销赋值

    if (verified_depth_[current_variable] != -1) {
      if (store_[current_variable] == 1) ++mine_count;
      if (change_variable_value(store_[current_variable], false)) {
        calculate_solution(depth + 1);  // 计算下一层
      } // 将now在方程里正式赋值，发现赋完值以后没有出现矛盾
      if (store_[current_variable] == 1) --mine_count;
      change_variable_value(store_[current_variable], true);
    } // 值已经被确定了。此时now应该还没有在方程里正式地赋值
    else {
      bool is_reverse = (unknown_mines * 1.0 / (unknown_blocks * 1.0) > 0.55); // 如果未知雷数更多，进行翻转，先钦定为雷
      verified_depth_[current_variable] = depth;

      // try 0 ^ is_reverse
      if (change_variable_value((store_[current_variable] = 0 ^ is_reverse), false)) {
        mine_count += (0 ^ is_reverse);
        calculate_solution(depth + 1);  // 计算下一层
        mine_count -= (0 ^ is_reverse);
      } // 将now在方程里赋为0 ^ is_reverse，发现赋完值以后没有出现矛盾

      change_variable_value(store_[current_variable], true);
      // try 1 ^ is_reverse
      if (change_variable_value((store_[current_variable] = 1 ^ is_reverse), false)) {
        mine_count += (1 ^ is_reverse);
        calculate_solution(depth + 1);  // 计算下一层
        mine_count -= (1 ^ is_reverse);
      } // 将now在方程里赋为1 ^ is_reverse，发现赋完值以后没有出现矛盾
      
      change_variable_value(store_[current_variable], true);
      verified_depth_[current_variable] = -1;
    }
    return;
  } // 搜索解
  void gaussian_jordan() {
    // 以下，给每一个方程找主元，并进行消元
    for (int variable = 0; variable < variable_count_; ++variable) {
      for (int equation = 0; equation < equation_.size(); ++equation) {
        if (equation_[equation].main_variable_ != -1) continue; // 已经找到主元了
        auto it = equation_[equation].variable_.find(variable);
        if (it == equation_[equation].variable_.end()) continue; // 根本没有这个元
        if (std::abs(it->second) < EPSILON) {
          equation_[equation].variable_.erase(it);
          continue;
        }
        equation_[equation].main_variable_ = variable;
        main_equation_[variable] = equation;
        break;
      }
      if (main_equation_[variable] == -1) {
        variable_priority_[variable] = 0.0; // 自由元默认优先级
        free_variable_.push_back(variable);
        continue;
      }
      equation_[main_equation_[variable]] /= equation_[main_equation_[variable]][variable]; // 系数归一化
      for (int equation = 0; equation < equation_.size(); ++equation) {
        if (equation == main_equation_[variable]) continue; // 保留住自己
        auto it = equation_[equation].variable_.find(variable);
        if (it == equation_[equation].variable_.end()) continue; // 不必消元
        if (std::abs(it->second) < EPSILON) {
          equation_[equation].variable_.erase(it);
          continue;
        } // 不必消元
        equation_[equation] -= (equation_[equation][variable] * equation_[main_equation_[variable]]); // 消元
      }
    } // 选择主元

    // 以下，统计包含variable的方程的编号
    for (int equation = 0; equation < equation_.size(); ++equation) {
      for (int variable = 0; variable < variable_count_; ++variable) {
        auto it = equation_[equation].variable_.find(variable);
        if (it == equation_[equation].variable_.end()) continue;  // 没有这个元
        if (std::abs(it->second) < EPSILON) {
          equation_[equation].variable_.erase(it);
          continue;
        }
        in_which_equations_[variable].push_back(equation);
      }
    }

    // 以下，计算自由元的枚举优先级
    for (int variable = 0; variable < variable_count_; ++variable) {
      if (main_equation_[variable] == -1) {
        double value = in_which_equations_[variable].size() * 1.0; // 优先搜索涉及到的方程更多的变量
        variable_priority_[variable] += value;
      }
      else {
        variable_priority_[variable] += in_which_equations_[variable].size() * 1.0;
      }
    }
    sort(enumeration_order_.begin(), enumeration_order_.end(),
         [&](int a, int b) -> bool { return variable_priority_[a] > variable_priority_[b]; });
    // 这样一来，所有的能消的就都消了，自由元也都存好了（都在前面，且已按权重排序）
    return;
  } // 高斯消元
  void clear() {
    variable_count_ = 0;
    solution_count_ = 0;
    equation_.clear();
    main_equation_.clear();
    node_set_.clear();
    variable_priority_.clear();
    enumeration_order_.clear();
    free_variable_.clear();
    store_.clear();
    zero_count_.clear();
    one_count_.clear();
    in_which_equations_.clear();
    verified_depth_.clear();
    return;
  } // 清空
  void add_block(int current_position) {
    UpdatePosition(current_position); // 添加到矩阵之前，首先更新它的信息
    if (block_status[current_position].is_not_worthy_analyzing() ||
       (block_status[current_position].unknown_block_count_ == 0)) return;
    node_set_.push_back(current_position);
    return;
  } // 添加数字方格
  bool manage_singular_solution() {
    if (solution_count_ != 1) return false;
    for (auto i : node_set_) {
      for (int j = 0; j < block_status[i].unknown_block_count_; ++j) {
        int this_one = block_status[i].unknown_block_position_[j];
        if (zero_count_[find_variable[this_one]]) {
          PushIntoOperationQueue(this_one, VISIT);
        }
        else {
          PushIntoOperationQueue(this_one, MARKMINE);
        }
      }
    }
    return true;
  } // 处理唯一解
  bool manage_multiple_solution() {
    bool is_find_possible_choice = false;
    for (auto i : node_set_) {
      for (int j = 0; j < block_status[i].unknown_block_count_; ++j) {
        int this_one = block_status[i].unknown_block_position_[j];
        int temporary_zero_count = zero_count_[find_variable[this_one]];
        int temporary_one_count = one_count_[find_variable[this_one]];
        int totalc = temporary_zero_count + temporary_one_count;
        if (!totalc) continue;
        if (temporary_zero_count == totalc) {
            PushIntoOperationQueue(this_one, VISIT);
            is_find_possible_choice = true;
        }
        else if (temporary_one_count == totalc) {
          PushIntoOperationQueue(this_one, MARKMINE);
          is_find_possible_choice = true;
        }
      }
    }
    return is_find_possible_choice;
  } // 如果多解，找到操作确定的位置
  bool manage_multiple_solution_maximize(int &current_position,
                                         int &operation_type, 
                                         double &maximum_possibility) {
    bool is_find_possible_choice = false;
    for (auto i : node_set_) {
      for (int j = 0; j < block_status[i].unknown_block_count_; ++j) {
        int this_one = block_status[i].unknown_block_position_[j];
        int temporary_zero_count = zero_count_[find_variable[this_one]];
        int temporary_one_count = one_count_[find_variable[this_one]];
        int totalc = temporary_zero_count + temporary_one_count;
        if (!totalc) continue;
        double possibility = std::max((temporary_zero_count * 1.0 / totalc), 
                                      (temporary_one_count * 1.0 / totalc));
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
    return true;
  } // 推断最大概率
 public:
  bool analyze(std::vector<int> positions,      // 涉及的方格
               bool &recommend_solution,        // 是否成功进行了推荐（如果为false，以下三个参数没有意义）
               int &current_position,           // 推荐进行操作的位置
               int &operation_type,             // 推荐进行操作的类型 
               double &maximum_possibility) {   // 推荐理由（最大可能性）
    clear();
    for (auto i : positions) {
      add_block(i);
    }
    set_up_matrix();
    gaussian_jordan();
    mine_count = 0;
    calculate_solution(0);
    if (manage_singular_solution()) return true;
    if (manage_multiple_solution()) return true;
    recommend_solution = manage_multiple_solution_maximize(current_position, 
                                                           operation_type, 
                                                           maximum_possibility);
    return false;
  } // 对给定的格子集合进行分析
} temporary_matrix;

Matrix global_matrix;
std::vector<int> global_vector;
bool GaussianElimination(int S_x, int S_y, int T_x, int T_y,
                         bool &recommend_solution,          // 是否成功进行了推荐（如果为false，以下三个参数没有意义）
                         int &current_position,             // 推荐进行操作的位置
                         int &operation_type,               // 推荐进行操作的类型
                         double &maximum_possibility) {     // 推荐理由（最大可能性）
  recommend_solution = false;
  maximum_possibility = -1.0;
  global_vector.clear();
  for (int i = S_x; i <= T_x; ++i) {
    for (int j = S_y; j <= T_y; ++j) {
      int current_position = Block::encode(i, j);
      if ((!block_status[current_position].is_not_worthy_analyzing()) && 
           block_status[current_position].unknown_block_count_ > 0) {
        global_vector.push_back(current_position);
      }
    }
  }
  if (global_vector.empty()) return false;
  return global_matrix.analyze(global_vector, 
                               recommend_solution, 
                               current_position, 
                               operation_type, 
                               maximum_possibility);
} // 对(S_x, S_y)为左上角，(T_x, T_y)为右下角的矩阵进行高斯消元推断

void Randomize() {
  for (int range = 5; range >= 0; --range) {
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
} // 随机访问，贪心寻找周围未访问的格子最密集的位置

void Analyze(int current_position) {
  if (block_status[current_position].is_not_worthy_analyzing())
    return; // 没有分析价值

  Pii position = Block::decode(current_position);
  int position_x = position.first, position_y = position.second;
  UpdatePosition(current_position);

  if (block_status[current_position].unknown_block_count_ == 0) {
    block_status[current_position].is_all_confirmed_ = true;
    return;
  } // 不存在未知格子，处理完毕
  else if (block_status[current_position].unknown_mine_count_ == 0) {
    block_status[current_position].is_all_confirmed_ = true;
    PushIntoOperationQueue(current_position, AUTOEXPLORE);
    return;
  } // 有不确定的位置但是雷的位置已经全定了，可以直接探索
  else if (block_status[current_position].unknown_mine_count_ == block_status[current_position].unknown_block_count_) {
    block_status[current_position].is_all_confirmed_ = true;
    for (int i = 0; i < block_status[current_position].unknown_block_count_; ++i) {
      PushIntoOperationQueue(block_status[current_position].unknown_block_position_[i], MARKMINE);
    }
    return;
  } // 不确定的位置 = 不确定的雷，全部标雷

  for (int enumerate_delta = 0; enumerate_delta < DELTA_FIVE; ++enumerate_delta) {
    int next_x = position_x + delta_row[enumerate_delta];
    int next_y = position_y + delta_column[enumerate_delta];
    if (!IsInMap(next_x, next_y)) continue;
    int next_position = Block::encode(next_x, next_y);
    UpdatePosition(next_position);
    if (block_status[next_position].is_not_worthy_analyzing()) continue;
    // 只需要和已经打开的、不是雷的、还存在待定位置的点进行小型高斯消元
    bool filler_a;
    double filler_b;
    int filler_c, filler_d;
    // 以上全是临时数据接口，属于填充物，不具备实际意义
    temporary_matrix.analyze({current_position, next_position}, 
                             filler_a, filler_c, filler_d, filler_b);
  } // 与周围的24个点进行小型高斯消元
  return;
} // 进行小型推断
bool GlobalScan(int x_len, int y_len) {
  bool flag = false;
  bool global_recommend_solution = false;
  double global_maximum_possibility = -1.0;
  int global_operation_type;
  int global_current_position;

  // 枚举大矩阵时，适当减小步进量，实现精细化搜索
  for (int i = 0; i < rows; i += (x_len >> 1)) {
    for (int j = 0; j < columns; j += (y_len >> 1)) {
          bool recommend_solution = false;
          double maximum_possibility = -1.0;
          int operation_type;
          int current_position;

          int T_x = std::min(i + x_len - 1, rows - 1);
          int T_y = std::min(j + y_len - 1, columns - 1);

          flag |= GaussianElimination(i, j, T_x, T_y, 
                                      recommend_solution,
                                      current_position, 
                                      operation_type, 
                                      maximum_possibility); // 记录唯一解情况

          if (recommend_solution) {
            global_recommend_solution = true;
            if (maximum_possibility > global_maximum_possibility) {
              global_maximum_possibility = maximum_possibility;
              global_operation_type = operation_type;
              global_current_position = current_position;
            }
          } // 记录推荐情况
        }
  }
  if ((!flag) && (global_recommend_solution)) {
    PushIntoOperationQueue(global_current_position, global_operation_type);
  } // 如果没有找到唯一解，就试图进行推荐
  return flag | global_recommend_solution;
} // 全局大矩阵扫描

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
void Decide() {
  while (true) {
    if (!operation_queue.empty()) {
      Option get_option = operation_queue.front();
      operation_queue.pop();
      Pii current_position = Block::decode(get_option.position_);
      Execute(current_position.first, current_position.second, get_option.type_);
      return;
    } // 执行可执行操作

    int position_in_front = GetPositionInTheFront();
    if (position_in_front == -1) {
      // 全局大矩阵扫描 + 随机访问
      if (GlobalScan(15, 15)) continue;
      Randomize();
    } // 没有待分析的点
    else {
      Analyze(position_in_front);
    } // 分析待分析的点
  }
}

#endif