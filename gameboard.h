#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <QObject>
#include <QAbstractListModel>

#include "struct.h"
#include "database.h"

class gameBoard:public QAbstractListModel
{
    Q_OBJECT
public:
    Q_PROPERTY(int currentScore READ currentScore WRITE setCurrentScore NOTIFY currentScoreChanged)
    Q_PROPERTY(bool isFinal READ isFinal WRITE setIsFinal NOTIFY isFinalChanged)

    enum circleRoles
    {
        cellColor = Qt::UserRole + 1,
        cellIsBusy
    };
    gameBoard(QObject* parent=0);
    ~gameBoard();
    dataBase* appDb = nullptr;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QHash<int, QByteArray> roleNames() const override;

    int currentScore() const;
    bool isFinal() const;

public slots:
    void refresh();
    void newGame();
    void clearBoard();
    bool tryFillBoardFromDB();
    bool tryFillInformationFromDB();
    void fillBoardEmptyCells();
    /*Проверяет на доступность ячейки, откуда игрок хочет переместить круг и запоминает её
     * в случае её доступности
    */
    bool tryToMakeAFirstMove(int index);/*Индекс ячейки откуда игрок хочет переместить круг*/
    /*Проверяет на доступность ячейку, куда хочет переместить круг игрок, и перемещает её в случае доступности
     *либо сбрасывает первый ход, елси ячейка недоступна
    */
    bool tryToMakeASecondMove(int index);
    //Заканчивает ход игрока проверяя на наличие победных линий и совершает ход компьютера
    void endASecondMove(int index);

    void setCurrentScore(int newScore);
    void setIsFinal(bool newIsFinal);

signals:
    void currentScoreChanged(int currentScore);
    void isFinalChanged(int isFinal);

private:
    QList<Cell> cells;
    QSet<int> freeCells;
    QHash<int, QByteArray> roles;

    int m_currentScore {0};
    bool m_isFinal {false};

    size_t pointsForWin = 10;
    size_t lehgthWin = 5;
    size_t maxRow = 9;
    size_t maxColumn = 9;
    size_t boardSize = maxRow*maxColumn;

    int firstClickCellId = -1;
    //Проверяет свободна ли ячейка
    bool checkCellIsFree(int index) const;
    //Меняет состояние ячейки на свободна
    void clearCell(int indexCell);
    //Размещает новую фигуру на поле
    void placeCell(int indexCell, int idColor);
    //Перемещает ячейку
    void moveCell(int indexFrom, int indexTo);
    //Проверяет что индекс не выходит из допустимого диапозона
    bool isValidPosition(int index) const;
    //Рекурсивная функция определяет, существует ли путь от from к to
    bool checkPossibilityWay(int from, int to, QSet<int>& cellsSeen) const;
    //Проверяет и применяет победные линии относительно заданного индекса
    void checkAndApplyWinLines(int index);
    //Проверяет горизонтальную линию на наличие 5 одинаковых ячеек подряд, с заданным цветом
    int checkHorLine(int index, ColorEnum needColor) const;
    //Проверяет вертикальную линию на наличие 5 одинаковых ячеек подряд, с заданным цветом
    int checkVerLine(int index, ColorEnum needColor) const;
    //Приминяет горизонтальную победную линию, убирая необходимые ячейки и прибавляя счёт
    void applyHorLine(int index);
    //Приминяет вертикальную победную линию, убирая необходимые ячейки и прибавляя счёт
    void applyVerLine(int index);
    //Ход компьютера
    void makeComputerMove();
    //Проверяет закончена ли игра
    bool checkIsFinal() const;


    //Возвращает индексы свободных клеток справа, слева, вверху и внизу от заданного индекса
    QList<int> getFreeNeighbourCells(int index) const;
    int getIndexRightNeighbour(int index) const;
    int getIndexLeftNeighbour(int index) const;
    int getIndexTopNeighbour(int index) const;
    int getIndexBottomNeighbour(int index) const;
};

#endif // GAMEBOARD_H
