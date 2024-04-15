#include <iostream>
#include <vector>
#include <algorithm> // Для использования std::find
#include <windows.h>
#include <string>

#define testAlpha
#define alphaBet
#define depthMinMax 7 // нечётное число

#ifdef testAlpha
int callsMiniMax = 0;
#endif

// Размер доски
const int BOARD_SIZE = 8;

// Структура, представляющая передвижение шашки
struct Jump
{
   int x1, y1;      // Начальная позиция
   int x2, y2;      // Конечная позиция
   bool isJumpOver; // если перепрыгнул шашку
   int x_d, y_d;    // Перепрыгнутая шашка

// Конструктор с параметрами
   Jump(int startX, int startY, int endX, int endY)
         : x1(startX), y1(startY), x2(endX), y2(endY)
   {
      isJumpOver = false;
      x_d = y_d = -1;
   }

// Конструктор с параметрами
   Jump(int startX, int startY, int endX, int endY, int deleteX, int deleteY)
         : x1(startX), y1(startY), x2(endX), y2(endY), isJumpOver(isJumpOver), x_d(deleteX), y_d(deleteY)
   {
      isJumpOver = true;
   }

};

// Структура, представляющая ход
struct Move
{
   std::vector<Jump> jumps;

   // Конструктор с вектором прыжков
   Move(const std::vector<Jump> jumps) : jumps(jumps)
   {}
};

// Перечисление цветов шашек
enum Color
{
   WHITE,
   BLACK
};

// Перечисление статуса игры
enum GameStatus
{
   ONGOING,     // Игра продолжается
   PLAYER1_WIN, // Победа игрока 1
   PLAYER2_WIN, // Победа игрока 2
   DRAW,        // Ничья
};

// Класс, представляющий игровое поле
class Board
{
public:
   char board[BOARD_SIZE][BOARD_SIZE] =
         {{'_', 'b', '_', 'b', '_', 'b', '_', 'b'},
          {'b', '_', 'b', '_', 'b', '_', 'b', '_'},
          {'_', 'b', '_', 'b', '_', 'b', '_', 'b'},
          {'_', '_', '_', '_', '_', '_', '_', '_'},
          {'_', '_', '_', '_', '_', '_', '_', '_'},
          {'w', '_', 'w', '_', 'w', '_', 'w', '_'},
          {'_', 'w', '_', 'w', '_', 'w', '_', 'w'},
          {'w', '_', 'w', '_', 'w', '_', 'w', '_'}};

   // Создание нового объекта Board
   Board newBoard() const
   {
      // Создаем копию текущего объекта Board
      Board newBoard = *this;
      return newBoard;
   }

   // Создание нового объекта Board с примененным ходом
   Board newBoard(Move move) const
   {
      // Создаем копию текущего объекта Board
      Board newBoard = *this;

      // Применяем переданный ход к новой доске
      newBoard.applyMove(move);

      return newBoard;
   }

   // Применение хода к текущему объекту Board
   void applyMove(Move move)
   {
      for (const auto &jump: move.jumps)
      {
         // Получаем начальную и конечную позиции прыжка
         int startX = jump.x1;
         int startY = jump.y1;
         int endX = jump.x2;
         int endY = jump.y2;

         // Обновляем конечную клетку
         board[endX][endY] = board[startX][startY];

         // Обновляем статус начальной клетки
         board[startX][startY] = '_';

         // проверка на дамку
         if (endX == 0 && board[endX][endY] > 96)
         {
            board[endX][endY] = board[endX][endY] - 32;
         }
         // Если при ходе перепрыгнули шашку, удаляем шашку
         if (jump.isJumpOver)
         {
            int deletedX = jump.x_d;
            int deletedY = jump.y_d;
            board[deletedX][deletedY] = '_';
         }
      }
   }

   void reverse()
   {
      for (int i = 0; i < BOARD_SIZE / 2; ++i)
      {
         for (int j = 0; j < BOARD_SIZE; ++j)
         {
            // переворачиваем клетки
            char temp = board[i][j];
            board[i][j] = board[BOARD_SIZE - 1 - i][BOARD_SIZE - 1 - j];
            board[BOARD_SIZE - 1 - i][BOARD_SIZE - 1 - j] = temp;
         }
      }
   }

   void print() const
   {
      HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
      std::cout << "\n  A B C D E F G H";
      SetConsoleTextAttribute(handle, 7);
      std::cout << '\n';
      for (int i = 0; i < 8; i++)
      {
         std::cout << i << " ";
         for (int j = 0; j < 8; j++)
         {
            if (!((i + j) % 2))
            {
               SetConsoleTextAttribute(handle, 240);
            } else
            {
               SetConsoleTextAttribute(handle, 7);
            }
            if (board[i][j] == 'b')
            {
               SetConsoleTextAttribute(handle, 11);
               std::cout << "()";
            } else if (board[i][j] == 'B')
            {
               SetConsoleTextAttribute(handle, 11);
               std::cout << "@@";
            } else if (board[i][j] == 'w')
            {
               SetConsoleTextAttribute(handle, 4);
               std::cout << "()";
            } else if (board[i][j] == 'W')
            {
               SetConsoleTextAttribute(handle, 4);
               std::cout << "@@";
            } else if (board[i][j] == '_')
            {
               std::cout << "  ";
            }
         }
         SetConsoleTextAttribute(handle, 7);
         std::cout << "\n";
      }
   }

   // Метод, проверяющий, находится ли ячейка на доске
   bool isCellExists(int row, int col) const
   {
      return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
   }

   // получить ход белой шашки
   void getMoveWhite(Board board, int row, int col, std::vector<Move> &Moves,
                     std::vector<std::pair<int, int>> cellForDelete,
                     std::vector<Jump> jumps, bool isJumpPrev = false) const
   {
      bool isJumpOver = false; // Флаг, указывающий, была ли шашка перепрыгнута
      bool isJump = false;     // Флаг, указывающий, был ли сделан прыжок

      for (int dr = -1; dr <= 1; dr += 2)
      {
         for (int dc = -1; dc <= 1; dc += 2)
         {
            int newRow = row + dr;
            int newCol = col + dc;
            int jumpRow = row + 2 * dr;
            int jumpCol = col + 2 * dc;
            // Проверяем, возможен ли прыжок
            if (board.isCellExists(jumpRow, jumpCol) &&
                board.board[jumpRow][jumpCol] == '_' &&
                (board.board[newRow][newCol] == 'b' ||
                 board.board[newRow][newCol] == 'B') &&
                std::find(cellForDelete.begin(), cellForDelete.end(), std::make_pair(newRow, newCol)) ==
                cellForDelete.end())
            {
               // Добавляем удалёную шашку
               std::vector<std::pair<int, int>> newCellForDelete = cellForDelete;
               newCellForDelete.push_back(std::make_pair(newRow, newCol));
               // Добавляем прыжок
               std::vector<Jump> newJumps = jumps;
               newJumps.push_back(Jump(row, col, jumpRow, jumpCol, newRow, newCol));

               isJumpOver = true;
               isJumpPrev = true;

               // Если шашка становится дамкой
               if (jumpRow == 0)
               {
                  Board newBoard = board.newBoard();
                  newBoard.board[jumpRow][jumpCol] = 'W';
                  newBoard.board[row][col] = '_';
                  getMoveWhiteQueen(newBoard, jumpRow, jumpCol, Moves, newCellForDelete, newJumps, isJumpPrev);
               } else
               {
                  Board newBoard = board.newBoard();
                  newBoard.board[jumpRow][jumpCol] = 'w';
                  newBoard.board[row][col] = '_';
                  getMoveWhite(newBoard, jumpRow, jumpCol, Moves, newCellForDelete, newJumps, isJumpPrev);
               }
            }
         }
      }

      // Если не было перепрыгнуто шашек и не было сделано прыжков ранее
      if (!isJumpOver && !isJumpPrev)
      {
         int dr = -1;
         for (int dc = -1; dc <= 1; dc += 2)
         {
            int newRow = row + dr;
            int newCol = col + dc;

            // Проверяем возможные ходы
            if (board.isCellExists(newRow, newCol) && board.board[newRow][newCol] == '_')
            {
               isJump = true;

               // Добавляем прыжок
               std::vector<Jump> newJumps = jumps;
               newJumps.push_back(Jump(row, col, newRow, newCol));

               Move move(newJumps);
               if (!move.jumps.empty())
               {
                  Moves.push_back(move);
               }
            }
         }
      }

      // Если не было сделано прыжков и простых ходов
      if (!isJumpOver && !isJump)
      {
         Move move(jumps);
         if (!move.jumps.empty())
         {
            Moves.push_back(move);
         }
      }
   }

   // получить ход чёрной шашки
   void
   getMoveBlack(Board board, int row, int col, std::vector<Move> &Moves,
                std::vector<std::pair<int, int>> cellForDelete,
                std::vector<Jump> jumps, bool isJumpPrev = false) const
   {
      bool isJumpOver = false; // Флаг, указывающий, была ли шашка перепрыгнута
      bool isJump = false;     // Флаг, указывающий, был ли сделан прыжок

      for (int dr = -1; dr <= 1; dr += 2)
      {
         for (int dc = -1; dc <= 1; dc += 2)
         {
            int newRow = row + dr;
            int newCol = col + dc;
            int jumpRow = row + 2 * dr;
            int jumpCol = col + 2 * dc;

            // Проверяем, возможен ли прыжок
            if (board.isCellExists(jumpRow, jumpCol) &&
                board.board[jumpRow][jumpCol] == '_' &&
                (board.board[newRow][newCol] == 'w' ||
                 board.board[newRow][newCol] == 'W') &&
                std::find(cellForDelete.begin(), cellForDelete.end(), std::make_pair(newRow, newCol)) ==
                cellForDelete.end())
            {

               // Добавляем удалёную шашку
               std::vector<std::pair<int, int>> newCellForDelete = cellForDelete;
               newCellForDelete.push_back(std::make_pair(newRow, newCol));

               // Добавляем прыжок
               std::vector<Jump> newJumps = jumps;
               newJumps.push_back(Jump(row, col, jumpRow, jumpCol, newRow, newCol));

               isJumpOver = true;
               isJumpPrev = true;

               // Если шашка становится дамкой
               if (jumpRow == 0)
               {
                  Board newBoard = board.newBoard();
                  newBoard.board[jumpRow][jumpCol] = 'B';
                  newBoard.board[row][col] = '_';
                  getMoveBlackQueen(newBoard, jumpRow, jumpCol, Moves, newCellForDelete, newJumps, isJumpPrev);
               } else
               {
                  Board newBoard = board.newBoard();
                  newBoard.board[jumpRow][jumpCol] = 'b';
                  newBoard.board[row][col] = '_';
                  getMoveBlack(newBoard, jumpRow, jumpCol, Moves, newCellForDelete, newJumps, isJumpPrev);
               }
            }
         }
      }

      // Если не было перепрыгнуто шашек и не было сделано прыжков ранее
      if (!isJumpOver && !isJumpPrev)
      {
         int dr = -1;
         for (int dc = -1; dc <= 1; dc += 2)
         {
            int newRow = row + dr;
            int newCol = col + dc;

            // Проверяем возможные ходы
            if (board.isCellExists(newRow, newCol) && board.board[newRow][newCol] == '_')
            {
               isJump = true;

               // Добавляем прыжок
               std::vector<Jump> newJumps = jumps;
               newJumps.push_back(Jump(row, col, newRow, newCol));

               Move move(newJumps);
               if (!move.jumps.empty())
               {
                  Moves.push_back(move);
               }
            }
         }
      }

      // Если не было сделано прыжков и простых ходов
      if (!isJumpOver && !isJump)
      {
         Move move(jumps);
         if (!move.jumps.empty())
         {
            Moves.push_back(move);
         }
      }
   }

   // Получить ход белой дамки
   void getMoveWhiteQueen(Board board, int row, int col, std::vector<Move> &Moves,
                          std::vector<std::pair<int, int>> cellForDelete,
                          std::vector<Jump> jumps, bool isJumpPrev = false) const
   {
      bool isJumpOver = false;                   // Флаг, если перепрыгнула шашка
      std::vector<std::vector<Jump>> jumpsEmpty; // Список ходов на пустые клетки для дамки
      // Проходимся по диагонали дамки
      for (int dr = 1; dr > -2; dr -= 2)
      {
         for (int dc = 1; dc > -2; dc -= 2)
         {
            std::vector<std::vector<Jump>> jumpsOver; // Список ходов через шашки другого цвета
            bool isDoubleJump = false;                // Флаг, если перепрыгнули две шашки
            int r = row + dr;                         // Строка
            int c = col + dc;                         // Столбец

            // Записываем пустые клетки, пока они есть
            while (board.isCellExists(r, c) && board.board[r][c] == '_')
            {
               std::vector<Jump> newJumps = jumps;
               newJumps.push_back(Jump(row, col, r, c)); // Добавляем прыжок на пустую клетку
               jumpsEmpty.push_back(newJumps);
               r += dr;
               c += dc;
            }

            // Если нашлась чёрная клетка, которая не была съедена
            if (board.isCellExists(r, c) && (board.board[r][c] == 'b' || board.board[r][c] == 'B') &&
                std::find(cellForDelete.begin(), cellForDelete.end(), std::make_pair(r, c)) ==
                cellForDelete.end())
            {
               int r_del = r; // Строка шашки для удаления
               int c_del = c; // Столбец шашки для удаления
               r += dr;
               c += dc;
               // Пока есть пустые клетки после найдённой чёрной клетки
               while (board.isCellExists(r, c) && board.board[r][c] == '_')
               {
                  std::vector<Jump> newJumps = jumps;                     // Ход с перепрыгиванием
                  newJumps.push_back(Jump(row, col, r, c, r_del, c_del)); // Добавляем прыжок на пустую клетку
                  jumpsOver.push_back(newJumps);
                  // Проверяем, есть ли возможность перепрыгнуть другую шашку после перепрыгивания шашки (обязательный ход)
                  for (int drr = 1; drr > -2; drr -= 2)
                  {
                     for (int dcc = 1; dcc > -2; dcc -= 2)
                     {
                        int rr = r + drr;
                        int cc = c + dcc;

                        std::vector<std::pair<int, int>> newCellForDelete = cellForDelete;
                        newCellForDelete.push_back(
                              std::make_pair(r_del, c_del)); // Добавляем в вектор удаляемую шашку

                        // Перепрыгиваем пустые клетки
                        while (board.isCellExists(rr, cc) && board.board[rr][cc] == '_')
                        {
                           rr += drr;
                           cc += dcc;
                        }

                        // Если нашлась чёрная клетка, которая не была съедена
                        if (board.isCellExists(rr, cc) &&
                            (board.board[rr][cc] == 'b' || board.board[rr][cc] == 'B') &&
                            std::find(newCellForDelete.begin(), newCellForDelete.end(),
                                      std::make_pair(rr, cc)) == newCellForDelete.end())
                        {
                           rr += drr;
                           cc += dcc;

                           // Если следующая клетка после чёрной существует и является пустой
                           if (board.isCellExists(rr, cc) && board.board[rr][cc] == '_')
                           {
                              isJumpOver = true;   // Перепрыгнули шашку
                              isJumpPrev = true;   // Перепрыгивали раньше
                              isDoubleJump = true; // Перепрыгнули две шашки
                              Board newBoard = board.newBoard();
                              newBoard.board[r][c] = 'W';     // Новая позиция
                              newBoard.board[row][col] = '_'; // Старая позиция
                              getMoveWhiteQueen(newBoard, r, c, Moves, newCellForDelete, newJumps,
                                                isJumpPrev);
                           }
                        }
                     }
                  }
                  r += dr;
                  c += dc;
               }
            }

            // Если нету двойных прыжков для текущей диагонали
            if (!isDoubleJump && jumpsOver.size() != 0)
            {
               isJumpOver = true;
               isJumpPrev = true;
               for (std::vector<Jump> j: jumpsOver)
               {
                  Move move(j);
                  Moves.push_back(move);
               }
            }
         }
      }

      // Если не перепрыгивали шашки и есть ходы на пустые клетки
      if (!isJumpOver && !isJumpPrev && jumpsEmpty.size() != 0)
      {
         for (std::vector<Jump> j: jumpsEmpty)
         {
            Move move(j);
            Moves.push_back(move);
         }
      }

      // Если шашка стала дамкой и не ходила в роли дамки
      if (isJumpPrev && !isJumpOver && jumps.size() != 0)
      {
         Move move(jumps);
         Moves.push_back(move);
      }
   }

   void getMoveBlackQueen(Board board, int row, int col, std::vector<Move> &Moves,
                          std::vector<std::pair<int, int>> cellForDelete,
                          std::vector<Jump> jumps, bool isJumpPrev = false) const
   {
      bool isJumpOver = false;                   // Флаг, если перепрыгнула шашка
      std::vector<std::vector<Jump>> jumpsEmpty; // Список ходов на пустые клетки для дамки

      // Проходимся по диагонали дамки
      for (int dr = 1; dr > -2; dr -= 2)
      {
         for (int dc = 1; dc > -2; dc -= 2)
         {
            std::vector<std::vector<Jump>> jumpsOver; // Список ходов через шашки другого цвета
            bool isDoubleJump = false;                // Флаг, если перепрыгнули две шашки
            int r = row + dr;                         // Строка
            int c = col + dc;                         // Столбец

            // Записываем пустые клетки, пока они есть
            while (board.isCellExists(r, c) && board.board[r][c] == '_')
            {
               std::vector<Jump> newJumps = jumps;
               newJumps.push_back(Jump(row, col, r, c)); // Добавляем прыжок на пустую клетку
               jumpsEmpty.push_back(newJumps);
               r += dr;
               c += dc;
            }

            // Если нашлась белая клетка, которая не была съедена
            if (board.isCellExists(r, c) && (board.board[r][c] == 'w' || board.board[r][c] == 'W') &&
                std::find(cellForDelete.begin(), cellForDelete.end(), std::make_pair(r, c)) ==
                cellForDelete.end())
            {
               int r_del = r; // Строка шашки для удаления
               int c_del = c; // Столбец шашки для удаления
               r += dr;
               c += dc;

               // Пока есть пустые клетки после найдённой белой клетки
               while (board.isCellExists(r, c) && board.board[r][c] == '_')
               {
                  std::vector<Jump> newJumps = jumps;                     // Ход с перепрыгиванием
                  newJumps.push_back(Jump(row, col, r, c, r_del, c_del)); // Добавляем прыжок на пустую клетку
                  jumpsOver.push_back(newJumps);

                  // Проверяем, есть ли возможность перепрыгнуть другую шашку после перепрыгивания шашки (обязательный ход)
                  for (int drr = 1; drr > -2; drr -= 2)
                  {
                     for (int dcc = 1; dcc > -2; dcc -= 2)
                     {
                        int rr = r;
                        int cc = c;
                        rr += drr;
                        cc += dcc;

                        std::vector<std::pair<int, int>> newCellForDelete = cellForDelete;
                        newCellForDelete.push_back(
                              std::make_pair(r_del, c_del)); // Добавляем в вектор удаляемую шашку

                        // Перепрыгиваем пустые клетки
                        while (board.isCellExists(rr, cc) && board.board[rr][cc] == '_')
                        {
                           rr += drr;
                           cc += dcc;
                        }

                        // Если нашлась белая клетка, которая не была съедена
                        if (board.isCellExists(rr, cc) &&
                            (board.board[rr][cc] == 'w' || board.board[rr][cc] == 'W') &&
                            std::find(newCellForDelete.begin(), newCellForDelete.end(),
                                      std::make_pair(rr, cc)) == newCellForDelete.end())
                        {
                           rr += drr;
                           cc += dcc;

                           // Если следующая клетка после белой существует и является пустой
                           if (board.isCellExists(rr, cc) && board.board[rr][cc] == '_')
                           {
                              isJumpOver = true;   // Перепрыгнули шашку
                              isJumpPrev = true;   // Перепрыгивали раньше
                              isDoubleJump = true; // Перепрыгнули две шашки
                              Board newBoard = board.newBoard();
                              newBoard.board[r][c] = 'B';     // Новая позиция
                              newBoard.board[row][col] = '_'; // Старая позиция
                              getMoveBlackQueen(newBoard, r, c, Moves, newCellForDelete, newJumps,
                                                isJumpPrev);
                           }
                        }
                     }
                  }
                  r += dr;
                  c += dc;
               }
            }

            // Если нету двойных прыжков для текущей диагонали
            if (!isDoubleJump && jumpsOver.size() != 0)
            {
               isJumpOver = true;
               isJumpPrev = true;
               for (std::vector<Jump> j: jumpsOver)
               {
                  Move move(j);
                  Moves.push_back(move);
               }
            }
         }
      }

      // Если не перепрыгивали шашки и есть ходы на пустые клетки
      if (!isJumpOver && !isJumpPrev && jumpsEmpty.size() != 0)
      {
         for (std::vector<Jump> j: jumpsEmpty)
         {
            Move move(j);
            Moves.push_back(move);
         }
      }

      // Если шашка стала дамкой и не ходила в роли дамки
      if (isJumpPrev && !isJumpOver && jumps.size() != 0)
      {
         Move move(jumps);
         Moves.push_back(move);
      }
   }

   // Метод для получеия хода в зависимости от цвета
   void getMove(Board board, int row, int col, std::vector<Move> &Moves, Color color) const
   {
      if (color == Color::WHITE)
      {
         if (board.board[row][col] == 'w')
         {
            getMoveWhite(board, row, col, Moves, std::vector<std::pair<int, int>>(), std::vector<Jump>());
         } else if (board.board[row][col] == 'W')
         {
            getMoveWhiteQueen(board, row, col, Moves, std::vector<std::pair<int, int>>(), std::vector<Jump>());
         }
      } else
      {
         if (board.board[row][col] == 'b')
         {
            getMoveBlack(board, row, col, Moves, std::vector<std::pair<int, int>>(), std::vector<Jump>());
         } else if (board.board[row][col] == 'B')
         {
            getMoveBlackQueen(board, row, col, Moves, std::vector<std::pair<int, int>>(), std::vector<Jump>());
         }
      }
   }

   // Метод для получения ходов
   std::vector<Move> getMoves(Color color) const
   {
      std::vector<Move> possibleMoves; // возможные ходы для белых

      for (int i = 0; i < BOARD_SIZE; ++i)
      {
         for (int j = 0; j < BOARD_SIZE; ++j)
         {
            getMove(*this, i, j, possibleMoves, color);
         }
      }
      std::vector<Move> jumpOverMoves = getMovesWithJumpOver(possibleMoves);
      // проверяем, есть ли ходы с возможностью съесть чужие шашки (и/или) дамки
      if (!jumpOverMoves.empty())
      {
         return jumpOverMoves; // возвращаем список ходов, в которых обязаны съесть чужие шашки
      }
      return possibleMoves; // возвращаем возможные ходы
   }

   // Метод для получения ходов c перепрыгиванием
   std::vector<Move> getMovesWithJumpOver(std::vector<Move> Moves) const
   {
      std::vector<Move> jumpOverMoves;
      for (Move m: Moves)
      {
         for (Jump j: m.jumps)
         {
            if (j.isJumpOver)
            {
               jumpOverMoves.push_back(m);
               break;
            }
         }
      }
      return jumpOverMoves;
   }
};

// Оценочная функция для чёрных шашек (сверху, максимизация) и белых шашек (снизу, минимизация)
int Evaluation(Board &board)
{
   int eval = 0;            // оценка позиции
   int countLeftBlack = 0;  // количество чёрных шашек на левом фланге
   int countRigthBlack = 0; // количество чёрных шашек на правом фланге
   int countLeftWhite = 0;
   int countRigthWhite = 0;
   // (1) Материальное преимущество
   // вес шашки - 10
   // вес дамки - 30
   for (int i = 0; i < BOARD_SIZE; ++i)
   {
      for (int j = 0; j < BOARD_SIZE; ++j)
      {
         char c = board.board[i][j];
         if (c == 'b')
         {
            eval += 10;
            countLeftBlack++;
         } else if (c == 'B')
         {
            eval += 30;
            countRigthBlack++;
         } else if (c == 'w')
         {
            eval -= 10;
            countLeftWhite++;
         } else if (c == 'W')
         {
            eval -= 30;
            countRigthWhite++;
         }
      }
   }
   // (2) Взаимодействие флангов - нужно соблюдать
   // разность количества шашек на флангах у чёрных - это хорошо для белых
   int diffB = countLeftBlack - countRigthBlack;
   if (diffB < 0)
   {
      diffB = -diffB;
   }
   eval -= diffB * 6;

   // разность количества шашек на флангах у белых - это хорошо для чёрных
   int diffW = countLeftWhite - countRigthWhite;
   if (diffW < 0)
   {
      diffW = -diffW;
   }
   eval += diffW * 6;
   // (3) Избегать наличия бортовых шашек (ряды a, h), так как они ограничены в движениях.
   // доп. вес шашки на a и h - 5
   // доп. вес дамки на a и h - 10
   for (int i = 0; i < BOARD_SIZE; ++i)
   {
      for (int j = 0; j < BOARD_SIZE; j += 7)
      {
         char c = board.board[i][j];
         if (c == 'b')
         {
            eval -= 5;
         } else if (c == 'B')
         {
            eval -= 10;
         } else if (c == 'w')
         {
            eval += 5;
         } else if (c == 'W')
         {
            eval += 10;
         }
      }
   }
   // (4) Развивать отсталые шашки a1 и особенно h2 (белые), h8 и a7 (чёрные).
   // a1 - координаты (7, 0); h2 - координаты (6, 7)
   // h8 - координаты (0, 7); a7 - координаты (1, 0)
   // доп. вес шашек - 15
   if (board.board[7][0] == 'w' || board.board[7][0] == 'W')
   {
      eval += 15;
   }
   if (board.board[6][7] == 'w' || board.board[6][7] == 'W')
   {
      eval += 15;
   }
   if (board.board[0][7] == 'b' || board.board[0][7] == 'B')
   {
      eval -= 15;
   }
   if (board.board[1][0] == 'b' || board.board[1][0] == 'B')
   {
      eval -= 15;
   }
   // (5) Не торопиться с развитием шашек дамочного ряда (кроме a1, h8).
   // Особенно «золотой» шашки: в поддавках это c1 (белые) и f8 (чёрные),
   // так как она является ключевым звеном для выполнения различных комбинаций.
   // c1 - координаты (7,2); f8 - координаты (0,5)
   // доп. вес шашки - 14
   // доп. вес шашки на c1 и f8 - 7
   for (int j = 1; j < BOARD_SIZE; j += 2)
   {
      if (board.board[0][j] == 'b' || board.board[0][j] == 'B')
      {
         eval += 14;
      }
   }
   for (int j = 0; j < BOARD_SIZE; j += 2)
   {
      if (board.board[7][j] == 'w' || board.board[7][j] == 'W')
      {
         eval -= 14;
      }
   }
   if (board.board[7][2] == 'w' || board.board[7][2] == 'W')
   {
      eval -= 7;
   }
   if (board.board[0][5] == 'b' || board.board[0][5] == 'B')
   {
      eval += 7;
   }
   // (6) Занимать центральные поля доски доски: d4, f4, c5, e5.
   // Особенно выгодны e5 (белые) и d4 (чёрные).
   // d4 - (4,3); f4 - (4,5); e4 - (3,4); d4 - (3,2)
   // доп. вес шашки - 18
   // доп. вес шашки на e5 или d4 - 8
   // d4
   if (board.board[4][3] == 'w' || board.board[4][3] == 'W')
   {
      eval -= 18;
   }
   if (board.board[4][3] == 'b' || board.board[4][3] == 'B')
   {
      eval += 18;
   }
   // f4
   if (board.board[4][5] == 'w' || board.board[4][5] == 'W')
   {
      eval -= 18;
   }
   if (board.board[4][5] == 'b' || board.board[4][5] == 'B')
   {
      eval += 18;
   }
   // e5
   if (board.board[3][4] == 'w' || board.board[3][4] == 'W')
   {
      eval -= 26;
   }
   if (board.board[3][4] == 'b' || board.board[3][4] == 'B')
   {
      eval += 18;
   }
   // d4
   if (board.board[3][2] == 'w' || board.board[3][2] == 'W')
   {
      eval -= 18;
   }
   if (board.board[3][2] == 'b' || board.board[3][2] == 'B')
   {
      eval += 26;
   }
   return eval;
}

// минимакс функция для чёрных шашек
int MiniMax(Board &board, int depth, int a, int b, Color color)
{
#ifdef testAlpha
   ++callsMiniMax;
#endif
   std::vector<Move> moves = board.getMoves(color);
   if (moves.size() == 0)
   {
      if (color == Color::BLACK)
      {
         return INT_MAX;
      } else
      {
         return INT_MIN;
      }
   }
   if (depth == 0)
   {
      return Evaluation(board);
   }
   if (color == Color::BLACK)
   {
      int maxEval = INT_MIN;
      for (Move m: moves)
      {
         Board newBoard = board.newBoard(m);                    // создаём новую доску, с применённым ходом
         newBoard.reverse();                                    // переворачиваем доску для хода соперника
         int eval = MiniMax(newBoard, depth - 1, a, b, Color::WHITE); // получаем значение минимакса для хода
         if (maxEval < eval)
         {
            maxEval = eval;
         }
#ifdef alphaBet
         if (maxEval > a)
         {
            a = maxEval;
         }
         if (maxEval > b)
         {
            break;
         }
#endif
      }
      return maxEval;
   } else if (color == Color::WHITE)
   {
      int minEval = INT_MAX;
      for (Move m: moves)
      {
         Board newBoard = board.newBoard(m);                    // создаём новую доску, с применённым ходом
         newBoard.reverse();                                    // переворачиваем доску для хода соперника
         int eval = MiniMax(newBoard, depth - 1, a, b, Color::BLACK); // получаем значение минимакса для хода
         if (minEval > eval)
         {
            minEval = eval;
         }
#ifdef alphaBet
         if (minEval < b)
         {
            b = minEval;
         }
         if (minEval < a)
         {
            break;
         }
#endif
      }
      return minEval;
   }
   return 0;
}

// Интерфейс Player
class IPlayer
{
public:
   // Метод для получения индекса выбранного хода от игрокаY
   virtual int getIndexMove(Board &board, std::vector<Move> &moves) = 0;

   // Метод для оповещения игрока о конце игры
   virtual void End(Board &board, GameStatus gStat) = 0;
};

// Класс игрока
class Player : public IPlayer
{
   // Реализация методов интерфейса IPlayer
public:
   int getIndexMove(Board &board, std::vector<Move> &moves) override
   {
      // Реализация метода getIndexMove для игрока
      std::string inputStr = "-1";
      int exit = 0;
      while (true)
      {
#ifdef testAlpha
         std::cout << callsMiniMax << " numbers of calls miniMax" << '\n';
#endif
         board.print();
         int i = 1;
         for (const Move &move: moves)
         {
            std::cout << i << ". ";
            ++i;
            for (const Jump &jump: move.jumps)
            {
               std::cout << char('A' + jump.y1) << jump.x1 << "->" << char('A' + jump.y2) << jump.x2 << ',';
            }

            std::cout << std::endl;
         }
         std::cout << "Choose your move (1-" << i - 1 << "): ";
         std::cin >> inputStr; // ввод индекса хода пользовател
         try
         {
            exit = std::stoi(inputStr); // преобразование в инт
            if (exit == -1)   // -1 это досрочное завершение
            {
               return -1;
            }
            if (exit > 0 && exit < i) // если верный индекс хода
            {
               return exit - 1;
            } else
            {
               std::cout << "\n"
                         << "Your choose not correct, repeat your choose!" << '\n';
            }
         }
         catch (...)
         {
            std::cout << "\n"
                      << "Your choose not correct, repeat your choose!" << '\n';
         }
      }
   }

   void End(Board &board, GameStatus gameStatus) override
   {
      // Реализация метода End для игрока
      board.print();
      if (gameStatus == GameStatus::PLAYER1_WIN)
      {
         std::cout << '\n'
                   << "You Win!" << '\n';
      } else if (gameStatus == GameStatus::PLAYER2_WIN)
      {
         std::cout << '\n'
                   << "You lose!" << '\n';
      } else
      {
         std::cout << '\n'
                   << "ERROR" << '\n';
      }
   }
};

// Класс ИИ играет чёрными шашками
class AI : public IPlayer
{
   // Реализация методов интерфейса IPlayer
public:
   int getIndexMove(Board &board, std::vector<Move> &moves) override
   {
      // Реализация метода getIndexMove для ИИ
      int indexMove = 0;
      int maxEval = INT_MIN;
      int a = INT_MIN;
      int b = INT_MAX;
      for (int i = 0; i < moves.size(); ++i)
      {
         Board newBoard = board.newBoard(moves[i]);         // создаём новую доску, с применённым ходом
         newBoard.reverse();                                // переворачиваем доску для хода соперника
         int eval = MiniMax(newBoard, depthMinMax, a, b,
                            Color::WHITE); // получаем значение минимакса для хода
         if (maxEval > a)
         {
            a = maxEval;
         }
         if (maxEval < eval)
         {
            maxEval = eval;
            indexMove = i;
         }
      }
      return indexMove;
   }

   void End(Board &board, GameStatus GameStatus) override
   {
      // Реализация метода End для ИИ
   }
};

// Класс GameController
class GameController
{
private:
   IPlayer *player1;
   IPlayer *player2;
   Board *board;
   GameStatus gameStatus;

public:
   // Конструктор
   GameController(IPlayer *p1, IPlayer *p2) : player1(p1), player2(p2)
   {
      board = new Board();
      gameStatus = GameStatus::ONGOING;
   }

   // Метод для запуска игры
   void startGame()
   {
      // пока игра продолжается
      while (gameStatus == GameStatus::ONGOING)
      {
         // генерируем ходы для первого игрока
         std::vector<Move> moves = generateMoves(*board, Color::WHITE);
         if (moves.size() != 0)
         {
            // Первый игрок делает ход
            int index1 = player1->getIndexMove(*board, moves);
            // Обработка хода...
            if (index1 == -1) // -1 это досрочный проигрыш игрока 1
            {
               gameStatus = GameStatus::PLAYER2_WIN;
               break;
            }
            board->applyMove(moves[index1]);
         } else
         {
            gameStatus = GameStatus::PLAYER1_WIN;
            break;
         }
         // переворачиваем доску
         board->reverse();

         // генерируем ходы для второго игрока
         moves = generateMoves(*board, Color::BLACK);
         if (moves.size() != 0)
         {
            // Второй игрок делает ход
            int index2 = player2->getIndexMove(*board, moves);
            // Обработка хода...
            std::cout << "\nAI apply move: ";
            for (const Jump &jump: moves[index2].jumps)
            {
               std::cout << char('A' + 7 - jump.y1) << 7 - jump.x1 << "->" << char('A' + 7 - jump.y2)
                         << 7 - jump.x2 << ',';
            }
            std::cout << std::endl;
            board->applyMove(moves[index2]);
         } else
         {
            gameStatus = GameStatus::PLAYER2_WIN;
            break;
         }
         // переворачиваем доску
         board->reverse();
      }

      // Оповещение игроков о конце игры
      if (gameStatus == GameStatus::PLAYER2_WIN)
      {
         board->reverse();
      }
      player1->End(*board, gameStatus);
      player2->End(*board, gameStatus);
   }

private:
   // Метод для генерации доступных ходов
   std::vector<Move> generateMoves(Board board, Color color)
   {
      // Логика генерации ходов
      std::vector<Move> Moves = board.getMoves(color);
      return Moves;
   }
};

int main()
{
   std::cout << "To end the game enter (-1)";
   while (true)
   {
      std::string q;
      std::cout << '\n'
                << "Start new game? (y/n): ";
      std::cin >> q;
      if (q == "y")
      {
         IPlayer *me = new Player();
         IPlayer *ai = new AI();
         GameController game(me, ai);
         game.startGame();
      } else
      {
         break;
      }
   }
   return 0;
}