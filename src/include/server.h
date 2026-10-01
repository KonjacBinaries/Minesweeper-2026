#ifndef SERVER_H
#define SERVER_H

#include <cstdlib>
#include <iostream>
#include <queue>
#include <utility>

/*
 * You may need to define some global variables for the information of the game map here.
 * Although we don't encourage to use global variables in real cpp projects, you may have to use them because the use of
 * class is not taught yet. However, if you are member of A-class or have learnt the use of cpp class, member functions,
 * etc., you're free to modify this structure.
 */
const int dx[8] = {-1, -1, -1,  0,  0,  1,  1,  1}; //8 directions.
const int dy[8] = {-1,  0,  1, -1,  1, -1,  0,  1}; //8 directions.
char actual_state[35][35]; // actual map
char output_state[35][35]; // map for output
int normal_blocks; // The count of normal blocks.
int correctly_visited_normal_blocks; // The count of normal blocks that have been visited correctly.
int correctly_marked_mines; //The count of mines that are correctly marked.
std::queue<std::pair<int, int> > position_queue;
int rows;         // The count of rows of the game map. You MUST NOT modify its name.
int columns;      // The count of columns of the game map. You MUST NOT modify its name.
int total_mines;  // The count of mines of the game map. You MUST NOT modify its name. You should initialize this
                  // variable in function InitMap. It will be used in the advanced task.
int game_state;  // The state of the game, 0 for continuing, 1 for winning, -1 for losing. You MUST NOT modify its name.

/**
 * @brief The definition of function InitMap()
 *
 * @details This function is designed to read the initial map from stdin. For example, if there is a 3 * 3 map in which
 * mines are located at (0, 1) and (2, 2) (0-based), the stdin would be
 *     3 3
 *     .X.
 *     ...
 *     ..X
 * where X stands for a mine block and . stands for a normal block. After executing this function, your game map
 * would be initialized, with all the blocks unvisited.
 */
bool IsInMap(int x, int y) {
  return (0 <= x && x < rows) && (0 <= y && y < columns);
}
void InitMap() {
  std::cin >> rows >> columns;
  game_state = 0;
  total_mines = 0;
  correctly_marked_mines = 0;
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      std::cin >> actual_state[i][j];
      output_state[i][j] = '?';
      if (actual_state[i][j] == 'X') {
        ++total_mines;
      }
    }
  }
  normal_blocks = rows * columns - total_mines;
  correctly_visited_normal_blocks = 0;
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (actual_state[i][j] == 'X') {
        continue;
      }
      actual_state[i][j] = '0';
      for (int delta = 0; delta < 8; ++delta) {
        int new_x = i + dx[delta], new_y = j + dy[delta];
        if (!IsInMap(new_x, new_y)) continue;
        actual_state[i][j] += (actual_state[new_x][new_y] == 'X');
      }
    }
  }
  return;
}

/**
 * @brief The definition of function VisitBlock(int, int)
 *
 * @details This function is designed to visit a block in the game map. We take the 3 * 3 game map above as an example.
 * At the beginning, if you call VisitBlock(0, 0), game_state would be 0 (game continues), and the game map would
 * be
 *     1??
 *     ???
 *     ???
 * If you call VisitBlock(0, 1) after that, game_state would be -1 (game ends and the player loses), and the
 * game map would be
 *     1X?
 *     ???
 *     ???
 * If you call VisitBlock(0, 2), VisitBlock(2, 0), VisitBlock(1, 2) instead, game_state after the last operation
 * would be 1 (game ends and the player wins), and the game map would be
 *     1@1
 *     122
 *     01@
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 *
 * @note You should edit the value of game_state in this function. Precisely, edit it to
 *    0  if the game continues after visit that block, or that block has already been visited before.
 *    1  if the game ends and the player wins.
 *    -1 if the game ends and the player loses.
 *
 * @note For invalid operation, you should not do anything.
 */
void VisitBlock(int r, int c) {
  if (output_state[r][c] != '?') {
    return;
  } // 已经访问过了
  // 现在，(r, c)只能是'?'
  while (position_queue.size()) position_queue.pop();
  position_queue.push(std::make_pair(r, c));
  while (position_queue.size()) {
    std::pair<int, int> temp = position_queue.front();
    position_queue.pop();
    int pos_x = temp.first, pos_y = temp.second;
    if (output_state[pos_x][pos_y] != '?') continue;
    output_state[pos_x][pos_y] = actual_state[pos_x][pos_y];
    if (output_state[pos_x][pos_y] == 'X') {
      game_state = -1;
      return;
    } // 踩雷
    ++correctly_visited_normal_blocks; //多访问了一个
    if (correctly_visited_normal_blocks == normal_blocks) {
      correctly_marked_mines = total_mines;
      game_state = 1;
      return;
    } // 获胜
    // 现在，既没有踩雷，也没有获胜，游戏继续
    if (output_state[pos_x][pos_y] != '0') {
      continue;
    } // 只有0才会触发继续探索
    for (int dlt = 0; dlt < 8; ++dlt) {
      int new_x = pos_x + dx[dlt], new_y = pos_y + dy[dlt];
      if ((!IsInMap(new_x, new_y)) || (output_state[new_x][new_y] != '?')) continue;
      position_queue.push(std::make_pair(new_x, new_y));
    }
  }
  return;
}

/**
 * @brief The definition of function MarkMine(int, int)
 *
 * @details This function is designed to mark a mine in the game map.
 * If the block being marked is a mine, show it as "@".
 * If the block being marked isn't a mine, END THE GAME immediately. (NOTE: This is not the same rule as the real
 * game.)
 *
 * For example, if we use the same map as before, and the current state is:
 *     1?1
 *     ???
 *     ???
 * If you call MarkMine(0, 1), you marked the right mine. Then the resulting game map is:
 *     1@1
 *     ???
 *     ???
 * If you call MarkMine(1, 0), you marked the wrong mine(There's no mine in grid (1, 0)).
 * The game_state would be -1 and game ends immediately. The game map would be:
 *     1?1
 *     X??
 *     ???
 * This is different from the Minesweeper you've played. You should beware of that.
 *
 * @param r The row coordinate (0-based) of the block to be marked.
 * @param c The column coordinate (0-based) of the block to be marked.
 *
 * @note You should edit the value of game_state in this function. Precisely, edit it to
 *    0  if the game continues after visit that block, or that block has already been visited before.
 *    1  if the game ends and the player wins.
 *    -1 if the game ends and the player loses.
 *
 * @note For invalid operation, you should not do anything.
 */
void MarkMine(int r, int c) {
  if (output_state[r][c] != '?') return;
  if (actual_state[r][c] != 'X') {
    game_state = -1;
    output_state[r][c] = 'X';
    return;
  }
  ++correctly_marked_mines;
  output_state[r][c] = '@';
  return;
}

/**
 * @brief The definition of function AutoExplore(int, int)
 *
 * @details This function is designed to auto-visit adjacent blocks of a certain block.
 * See README.md for more information
 *
 * For example, if we use the same map as before, and the current map is:
 *     ?@?
 *     ?2?
 *     ??@
 * Then auto explore is available only for block (1, 1). If you call AutoExplore(1, 1), the resulting map will be:
 *     1@1
 *     122
 *     01@
 * And the game ends (and player wins).
 */
void AutoExplore(int r, int c) {
  if ((output_state[r][c] == '?') || (output_state[r][c] == '@')) return; // 只有数字才能explore
  while (position_queue.size()) position_queue.pop();
  for (int delta = 0; delta < 8; ++delta) {
    int new_x = r + dx[delta], new_y = c + dy[delta];
    if (!IsInMap(new_x, new_y)) continue;
    if ((actual_state[new_x][new_y] == 'X') && (output_state[new_x][new_y] != '@')) return; // 没有全部标记，拒绝操作
    if (output_state[new_x][new_y] == '?') position_queue.push(std::make_pair(new_x, new_y));
  }
  while (position_queue.size()) {
    std::pair<int, int> temp = position_queue.front();
    position_queue.pop();
    int pos_x = temp.first, pos_y = temp.second;
    if (output_state[pos_x][pos_y] != '?') continue;
    output_state[pos_x][pos_y] = actual_state[pos_x][pos_y];
    ++correctly_visited_normal_blocks; //多访问了一个
    if (correctly_visited_normal_blocks == normal_blocks) {
      game_state = 1;
      correctly_marked_mines = total_mines;
      return;
    } // 获胜
    // 现在，既没有踩雷，也没有获胜，游戏继续
    if (output_state[pos_x][pos_y] != '0') {
      continue;
    } // 只有0才会触发继续探索
    for (int delta = 0; delta < 8; ++delta) {
      int new_x = pos_x + dx[delta], new_y = pos_y + dy[delta];
      if ((!IsInMap(new_x, new_y)) || (output_state[new_x][new_y] != '?')) continue;
      position_queue.push(std::make_pair(new_x, new_y));
    }
  }
  return;
}

/**
 * @brief The definition of function ExitGame()
 *
 * @details This function is designed to exit the game.
 * It outputs a line according to the result, and a line of two integers, visit_count and marked_mine_count,
 * representing the number of blocks visited and the number of marked mines taken respectively.
 *
 * @note If the player wins, we consider that ALL mines are correctly marked.
 */
void ExitGame() {
  if (game_state == 1) {
    std::cout << "YOU WIN!\n";
    std::cout << correctly_visited_normal_blocks << ' ' << correctly_marked_mines << '\n';
  }
  else {
    std::cout << "GAME OVER!\n";
    std::cout << correctly_visited_normal_blocks << ' ' << correctly_marked_mines << '\n';
  }
  exit(0);  // Exit the game immediately
}

/**
 * @brief The definition of function PrintMap()
 *
 * @details This function is designed to print the game map to stdout. We take the 3 * 3 game map above as an example.
 * At the beginning, if you call PrintMap(), the stdout would be
 *    ???
 *    ???
 *    ???
 * If you call VisitBlock(2, 0) and PrintMap() after that, the stdout would be
 *    ???
 *    12?
 *    01?
 * If you call VisitBlock(0, 1) and PrintMap() after that, the stdout would be
 *    ?X?
 *    12?
 *    01?
 * If the player visits all blocks without mine and call PrintMap() after that, the stdout would be
 *    1@1
 *    122
 *    01@
 * (You may find the global variable game_state useful when implementing this function.)
 *
 * @note Use std::cout to print the game map, especially when you want to try the advanced task!!!
 */
void PrintMap() {
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (game_state == 1 && output_state[i][j] == '?') std::cout << '@';
      else std::cout << output_state[i][j];
    }
    std::cout << std::endl;
  }
  return;
}

#endif
