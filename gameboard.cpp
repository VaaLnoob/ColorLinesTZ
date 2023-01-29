#include "gameboard.h"

#include <QRandomGenerator>

gameBoard::~gameBoard(){
}

gameBoard::gameBoard(QObject *parent)
    : QAbstractListModel (parent){
    roles[cellColor]  = "cellColor";
    roles[cellIsBusy] = "cellIsBusy";
}

QVariant gameBoard::data(const QModelIndex &index, int role) const{
    if (!index.isValid())
        return QVariant();
    QVariant retVal= "";
    Cell cell = cells.at(index.row());
    switch (role){
    case cellColor:
        retVal = cell.getStrColor();
        break;
    case cellIsBusy:
        retVal = cell.isBusy;
        break;
    }

    return retVal;
}

int gameBoard::rowCount(const QModelIndex &parent) const{
    Q_UNUSED(parent);
    return cells.count();
}

QHash<int, QByteArray> gameBoard::roleNames() const{
    return roles;
}

void gameBoard::clearBoard() {
    beginResetModel();
    for (int i = 0; i < cells.size(); ++i) {
        Cell& cell = cells[i];
        if (cell.isBusy)
            freeCells.insert(i);
        cell.color = ColorEnum::COLORLESS;
        cell.isBusy = false;
    }
    setCurrentScore(0);
    makeComputerMove();
    endResetModel();
}

void gameBoard::refresh() {
    if (tryFillBoardFromDB() && tryFillInformationFromDB()) {
        setIsFinal(checkIsFinal());
        return;
    }
    newGame();
}

void gameBoard::newGame() {
    appDb->clearTableInformation();
    appDb->clearTablePositions();
    appDb->insertNewBasicInfo(0);
    fillBoardEmptyCells();
    setCurrentScore(0);
    setIsFinal(false);
    makeComputerMove();
}

bool gameBoard::tryFillInformationFromDB() {
    QJsonArray arr = appDb->getLastInformation();
    if (arr.empty())
        return false;
    auto jObj = arr.at(0).toObject();
    setCurrentScore(jObj.value("score").toString().trimmed().toInt());
    return true;
}

bool gameBoard::tryFillBoardFromDB() {
    QJsonArray arr = appDb->getLastProgressPosition();
    if (arr.empty())
        return false;
    beginResetModel();
    cells.clear();
    freeCells.clear();
    for (int i = 0; i < arr.count(); ++i) {
        auto jObj = arr.at(i).toObject();
        Cell cell;
        cell.isBusy = jObj.value("is_busy_cell").toString().trimmed().toInt();
        if(cell.isBusy)
            cell.setColor(jObj.value("color_cell").toString().trimmed().toInt());
        cells.append(cell);
        if(!cell.isBusy)
            freeCells.insert(i);
    }
    endResetModel();
    return true;
}

void gameBoard::fillBoardEmptyCells() {
    beginResetModel();
    cells.clear();
    freeCells.clear();
    for (int i = 0; i < static_cast<int>(boardSize); ++i) {
        Cell cell;
        cell.color = ColorEnum::COLORLESS;
        cell.isBusy = false;
        cells.append(cell);
        freeCells.insert(i);
        appDb->insertNewPosition(i, cell);
    }
    endResetModel();
}

int gameBoard::currentScore() const {
    return m_currentScore;
}

bool gameBoard::isFinal() const {
    return m_isFinal;
}

void gameBoard::setCurrentScore(int newScore) {
    if (m_currentScore == newScore)
        return;

    m_currentScore = newScore;
    appDb->updateScore(0, newScore);
    emit currentScoreChanged(m_currentScore);
}

void gameBoard::setIsFinal(bool newIsFinal) {
    if (m_isFinal == newIsFinal)
        return;

    m_isFinal = newIsFinal;
    emit isFinalChanged(m_isFinal);
}

bool gameBoard::tryToMakeAFirstMove(int index) {
    if (firstClickCellId == -1 && !checkCellIsFree(index)) {
        firstClickCellId = index;
        return true;
    }
    return false;
}

bool gameBoard::tryToMakeASecondMove(int index) {
    if (firstClickCellId == -1) {
        return false;
    }
    QSet<int> cellsSeen;
    if (!checkCellIsFree(index) ||
            !checkPossibilityWay(firstClickCellId, index, cellsSeen)) {
        firstClickCellId = -1;
        return false;
    }
    moveCell(firstClickCellId, index);
    firstClickCellId = -1;
    return true;
}

void gameBoard::endASecondMove(int index) {
    checkAndApplyWinLines(index);
    makeComputerMove();
}

bool gameBoard::isValidPosition(int index) const {
    return index >= 0 && index < cells.size();
}

int gameBoard::checkVerLine(int index, ColorEnum needColor) const {
    int indexStartVert = index;
    int resV = indexStartVert/static_cast<int>(maxRow);
    if (resV > 0)
        indexStartVert -= (indexStartVert/static_cast<int>(maxColumn)) * static_cast<int>(maxColumn);
    int maxLengthLine = 0;
    int indexFirstLine = -1;
    for (int i = indexStartVert; i < cells.size(); i+=maxColumn) {
        if (cells.at(i).isBusy && cells.at(i).color == needColor) {
            if (indexFirstLine == -1)
                indexFirstLine = i;
            maxLengthLine++;
        }
        else {
            maxLengthLine = 0;
            indexFirstLine = -1;
        }

        if (maxLengthLine == static_cast<int>(lehgthWin))
            break;
    }
    if (maxLengthLine == static_cast<int>(lehgthWin))
        return indexFirstLine;
    else
        return -1;
}

int gameBoard::checkHorLine(int index, ColorEnum needColor) const {
    int indexStartHor = index;
    int resH = indexStartHor%static_cast<int>(maxColumn);
    if (resH != 0 || resH != static_cast<int>(maxColumn))
        indexStartHor -= resH;
    int maxLengthLine = 0;
    int indexFirstLine = -1;
    for (int i = indexStartHor; i < indexStartHor+static_cast<int>(maxColumn); ++i) {
        if (cells.at(i).isBusy && cells.at(i).color == needColor) {
            if (indexFirstLine == -1)
                indexFirstLine = i;
            maxLengthLine++;
        }
        else {
            maxLengthLine = 0;
            indexFirstLine = -1;
        }

        if (maxLengthLine == static_cast<int>(lehgthWin))
            break;
    }
    if (maxLengthLine == static_cast<int>(lehgthWin))
        return indexFirstLine;
    else
        return -1;
}

void gameBoard::applyHorLine(int index) {
    setCurrentScore(m_currentScore+static_cast<int>(pointsForWin));
    for (int i = index; i < index+static_cast<int>(lehgthWin); ++i) {
        clearCell(i);
    }
}

void gameBoard::applyVerLine(int index) {
    setCurrentScore(m_currentScore+static_cast<int>(pointsForWin));
    for (int i = 0, ind = index; i < static_cast<int>(lehgthWin); ++i, ind+=maxColumn) {
        clearCell(ind);
    }
}

void gameBoard::checkAndApplyWinLines(int index) {
    ColorEnum needColor = cells.at(index).color;
    int indexFirstVert = checkVerLine(index, needColor);
    if (indexFirstVert != -1)
        applyVerLine(indexFirstVert);
    int indexFirstHor = checkHorLine(index, needColor);
    if (indexFirstHor != -1)
        applyHorLine(indexFirstHor);
}

bool gameBoard::checkCellIsFree(int index) const {
    return !cells.at(index).isBusy;
}

void gameBoard::placeCell(int indexCell, int idColor) {
    Cell cell;
    cell.setColor(idColor);
    cell.isBusy = true;
    cells.replace(indexCell, cell);
    freeCells.remove(indexCell);
    appDb->updateStatusPosition(indexCell, cell);
    QModelIndex idx = index(indexCell, 0);
    emit dataChanged(idx, idx);
}

void gameBoard::clearCell(int indexCell) {
    Cell cell = cells.at(indexCell);
    cell.isBusy = false;
    cells.replace(indexCell, cell);
    freeCells.insert(indexCell);
    appDb->updateStatusPosition(indexCell, cell);
    QModelIndex idx = index(indexCell, 0);
    emit dataChanged(idx, idx);
}

void gameBoard::moveCell(int indexFrom, int indexTo) {
    cells.replace(indexTo, cells.at(indexFrom));
    clearCell(indexFrom);
    freeCells.remove(indexTo);
    QModelIndex idx = index(indexTo, 0);
    emit dataChanged(idx, idx);
    appDb->updateStatusPosition(indexTo, cells.at(indexTo));
}

void gameBoard::makeComputerMove() {
    QList<int> cellsCompMove;
    for (size_t i = 0; i < 3; ++i) {
        int step = QRandomGenerator::global()->bounded(freeCells.size());
        int idColor = QRandomGenerator::global()->bounded(4) + 1;
        int idCell = *(freeCells.begin()+step);
        placeCell(idCell, idColor);
        cellsCompMove.append(idCell);
    }
    for (int idCell : cellsCompMove) {
        checkAndApplyWinLines(idCell);
    }
    setIsFinal(checkIsFinal());
}

bool gameBoard::checkIsFinal() const {
    return freeCells.count() <= 0;
}

bool gameBoard::checkPossibilityWay(int from, int to, QSet<int>& cellsSeen) const {
    if (from == to) {
        return true;
    }
    bool isPossibility = false;
    cellsSeen.insert(from);
    for (int ind : getFreeNeighbourCells(from)) {
        if (!cellsSeen.contains(ind))
            isPossibility = checkPossibilityWay(ind, to, cellsSeen);
        if (isPossibility)
            break;
    }
    return isPossibility;
}

QList<int> gameBoard::getFreeNeighbourCells(int index) const {
    QList<int> cellsRes;
    int rInd = getIndexRightNeighbour(index);
    if (rInd != -1 && !cells.at(rInd).isBusy)
        cellsRes.append(rInd);
    int lInd = getIndexLeftNeighbour(index);
    if (lInd != -1 && !cells.at(lInd).isBusy)
        cellsRes.append(lInd);
    int tInd = getIndexTopNeighbour(index);
    if (tInd != -1 && !cells.at(tInd).isBusy)
        cellsRes.append(tInd);
    int bInd = getIndexBottomNeighbour(index);
    if (bInd != -1 && !cells.at(bInd).isBusy)
        cellsRes.append(bInd);
    return cellsRes;
}

int gameBoard::getIndexRightNeighbour(int index) const {
    int ind = index;
    ind += 1;
    if (!isValidPosition(ind) || ind % static_cast<int>(maxColumn) == 0)
        ind = -1;
    return ind;
}

int gameBoard::getIndexLeftNeighbour(int index) const {
    int ind = index;
    ind -= 1;
    if (!isValidPosition(ind) || index % static_cast<int>(maxColumn) == 0)
        ind = -1;
    return ind;
}

int gameBoard::getIndexTopNeighbour(int index) const {
    int ind = index;
    ind -= maxColumn;
    if (!isValidPosition(ind))
        ind = -1;
    return ind;
}

int gameBoard::getIndexBottomNeighbour(int index) const {
    int ind = index;
    ind += maxColumn;
    if (!isValidPosition(ind))
        ind = -1;
    return ind;
}
