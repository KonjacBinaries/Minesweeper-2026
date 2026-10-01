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
    1.int encode(int x, int y)与pair<int, int> decode(int pos)
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
      试图将pos加入到待处理队列pos_queue中，维护一个in_pos_queue[]数组判断pos是不是已经在队里了
      如果pos的in_map(pos) == false或in_pos_queue[pos] == true或is_open == false或is_bomb == true或is_done == yes，那么驳回请求
      否则，把pos push到pos_queue里，标记in_pos_queue[pos] = true;
    2.int get_front_pos(int pos)
      如果pos_queue为空，返回-1
      否则，返回pos_queue.front()，弹出队列，标记in_pos_queue[pos] = false
    3.push_into_op_queue
      注意到每个点最多进行一次visit、一次explore、一次markmine（实际上不可能都进行），所以维护
      is_visited is_explored is_markmined
      来判断是不是应该入队
      把操作加入op_queue，等待执行
  四、操作执行组
    1.void explore(int pos) 任务是，explore pos，然后处理地图变动（维护好pos_queue）
      (1)Execute一下pos（进行explore）
      (2)Update_map();
    2.void markmine(int pos)
      (1)Execute一下pos（进行markmine）
      (2)Update_map();
    3.void visit(int pos)
      (1)Execute一下pos（进行visit）
      (2)Update_map();
    4.void cooperate(int pos, int pos')
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
        通过推式子，得到
        max(un_bomb - |A|, un_bomb' - |B|, 0) == min(un_bomb, un_bomb', |N|)
        是有唯一解的条件
        如果有唯一解，但是唯一解没有填满或置空A、N、B中的任何一个，仍然取消交集合作
        把A、N、B中，被填满或被置空的集合中的点打包成(pos, markmine)或打包成(pos, visit)然后push_into_op_queue
        这样一来，像 1 4，2 5之类的就都秒了
        的时候才可能有唯一解，这个值就是唯一的解
      return;
  五、狗急跳墙组
    1.void gaussian_elimination() 逼急了可以用
    2.void random_position() 如果发现gaussian的规模过大，就直接random
  */