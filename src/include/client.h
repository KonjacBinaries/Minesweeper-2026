#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

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

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the client state. It should be called at the beginning of the game,
 * after InitMap() has read the map scale. It reads and executes the first step provided by the input (see README).
 */
void InitGame() {
  // TODO (student): Initialize all your global variables!
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
  // TODO (student): Implement me!
}

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
void Decide() {
  /*
  对于每个节点，维护“该点是否已确定（空/雷）”，“是否是雷”，“周围八个点是否已处理完毕”，“周围8格中未确定有没有雷的格子数(记录坐标，通过Encode与Decode将二维坐标一维化)”，“周围8格中未确定的雷数”
                        is_open              is_bomb          is_done                   un_block_cnt        un_block_pos[8]                                       un_bomb
  Idea:
  一、核心功能组
    1.void Analyze(int pos)
      (1)is_open? no -> return; yes -> continue.
      (2)is_bomb? yes -> return; no -> continue.
      (3)is_done? yes -> return; no -> continue.
      (4)update（需要实现：维护un_block_cnt，un_block_pos[8]和un_bomb），如果有信息改变，把周围所有的8个点push_into_pos_queue(pos)
      (5)un_block_cnt == 0? set is_done to true, return; not yet -> continue.
      (6)un_bomb == 0? set is_done to true，把(pos, explore) push_into_op_queue，return; not yet -> continue.
      (7)un_bomb == un_block_cnt? set is_done to true，对un_block_pos[8]中的所有点打包成(pos, markmine)然后push_into_op_queue，return; not yet -> continue.
      (8)枚举周围的8个点pos'，进行Cooperate(pos, pos')操作
      return;
    2.void Decide()
      (1)如果op_queue非空，那么先从op_queue中取一个操作（需要弹出），执行
        Explore或Markmine或Visit
        return;
      (2)试图get_front_pos
        failed ?
          (i) Gaussian Elimination(Pending)
          (ii) Random(Pending)
          返回(1);
        success -> continue
      (3)Analyze(pos)
      (4)返回(1)
    3.void ReadMap()
    4.void InitGame()
  二、基础保障组
    1.int Encode(int x, int y)与pair<int, int> Decode(int pos)
      一维二维互转
    2.bool in_map(int pos)
      判断pos在不在map里
    3.void Update_map()
      (1)读入新map，和原map对照
      (2)如果一个点是新开的点，那么把它的is_open改成true，is_mine改成false，is_done改成false，
        并且un_block_cnt改为-1，然后push_into_pos_queue，这样一来，等它Analyze的时候会在(4)处update，
        然后信息就会改变，然后周围的八个点就会自动试图更新，这样新点就很好地嵌入队列里了（如果是空点也会自动判done）
      (3)如果一个点是新标的雷，那么把它的is_open改成true（注意！），is_mine改成true，把周围所有的8个点push_into_pos_queue(pos)
      (4)用新map覆盖原map
  三、queue维护组
    1.void push_into_pos_queue(int pos)
      试图将pos加入到待处理队列pos_queue中，维护一个in_queue[]数组判断pos是不是已经在队里了
      如果pos的in_map(pos) == false或in_queue[pos] == true或is_open == false或is_bomb == true或is_done == yes，那么驳回请求
      否则，把pos push到pos_queue里，标记in_queue[pos] = true;
    2.int get_front_pos(int pos)
      如果pos_queue为空，返回-1
      否则，返回pos_queue.front()，弹出队列，标记in_queue[pos] = false
  四、操作执行组
    1.void Explore(int pos) 任务是，explore pos，然后处理地图变动（维护好pos_queue）
      (1)Execute一下pos（进行explore）
      (2)Update_map();
    2.void Markmine(int pos)
      (1)Execute一下pos（进行markmine）
      (2)Update_map();
    3.void Visit(int pos)
      (1)Execute一下pos（进行visit）
      (2)Update_map();
    4.void Cooperate(int pos, int pos')
      首先，求出pos和pos'的un_block_pos[8]的交集N，并且得到它们分别对N取差集以后的集合A B
      也就是说，A仅仅属于pos，B仅仅属于pos'，N同时属于A和B
      (1)差集合作
        因为(A + N)中有un_bomb个雷，(B + N)中有un_bomb'个雷，
        所以A - B中有un_bomb - un_bomb'个雷
        显然A - B最大可以取到|A|，最小可以取到-|B|
        所以，
          如果un_bomb - un_bomb' = |A|，那么对A中的所有点打包成(pos, markmine)然后push_into_op_queue，对B中的所有点打包成(pos, visit)然后push_into_op_queue
          如果un_bomb - un_bomb' = -|B|，那么对B中的所有点打包成(pos, markmine)然后push_into_op_queue，对A中的所有点打包成(pos, visit)然后push_into_op_queue
        这样一来，像1 2 1，1 2 2 1之类的就都秒了
      (2)交集合作
        直接枚举N中塞多少个雷，并使用un_bomb和un_bomb'来判断A和B中有多少雷
        如果无解，取消交集合作
        如果有多个解，也取消交集合作
        如果有唯一解，但是唯一解没有填满或置空A、N、B中的任何一个，仍然取消交集合作
        把A、N、B中，被填满或被置空的集合中的点打包成(pos, markmine)或打包成(pos, visit)然后push_into_op_queue
        这样一来，像 1 4，2 5之类的就都秒了
      return;
  五、狗急跳墙组
    1.void Gaussian_elimination() 逼急了可以用
    2.void Random() 如果发现gaussian的规模过大，就直接random
  */
  // TODO (student): Implement me!
  // while (true) {
  //   Execute(0, 0);
  // }
}

#endif
