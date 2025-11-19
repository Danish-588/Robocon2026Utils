const gridContainer = document.getElementById('grid-container');
const randomizeBtn = document.getElementById('randomize-btn');
const findPathBtn = document.getElementById('find-path-btn');
const scoreDisplay = document.getElementById('score-display');

// --- TUNABLE COSTS ---
const R2_REWARD_FRONT = 100;
const R1_PENALTIES = [-5, -10, -15, -20]; // Penalty for R1 in row 0, 1, 2, 3
const FAKE_PENALTY = -10000;
const R2_OVERLOAD_PENALTY = 75; // Penalty for having more than 2 R2 blocks
// NOTE: R2_REWARD_DIAGONAL is not used as paths must be straight.
const R2_REWARD_DIAGONAL = 50; 
// --------------------

const allItems = [
    { color: 'orange', text: 'R1' },
    { color: 'orange', text: 'R1' },
    { color: 'orange', text: 'R1' },
    { color: 'purple', text: 'R2' },
    { color: 'purple', text: 'R2' },
    { color: 'purple', text: 'R2' },
    { color: 'purple', text: 'R2' },
    { color: 'red', text: 'Fake' },
    { color: 'blank', text: '' },
    { color: 'blank', text: '' },
    { color: 'blank', text: '' },
    { color: 'blank', text: '' },
];

const edgeIndices = [0, 1, 2, 3, 5, 6, 8, 9, 10, 11];
const fourthRowIndices = [9, 10, 11];

function createGrid(items) {
    gridContainer.innerHTML = '';
    items.forEach(itemData => {
        const item = document.createElement('div');
        item.classList.add('grid-item', itemData.color);
        item.textContent = itemData.text;
        item.draggable = true;
        gridContainer.appendChild(item);
    });
}

function randomizeGrid() {
    const r1Items = allItems.filter(item => item.text === 'R1');
    const fakeItem = allItems.find(item => item.text === 'Fake');
    let remainingItems = allItems.filter(item => item.text !== 'R1' && item.text !== 'Fake');

    const newGridItems = new Array(12).fill(null);

    const availableEdgeIndices = [...edgeIndices];
    r1Items.forEach(item => {
        const randomIndex = Math.floor(Math.random() * availableEdgeIndices.length);
        const position = availableEdgeIndices.splice(randomIndex, 1)[0];
        newGridItems[position] = item;
    });

    const validFakePositions = [];
    for (let i = 0; i < 12; i++) {
        if (newGridItems[i] === null && !fourthRowIndices.includes(i)) {
            validFakePositions.push(i);
        }
    }

    if (validFakePositions.length > 0) {
        const fakePosition = validFakePositions[Math.floor(Math.random() * validFakePositions.length)];
        newGridItems[fakePosition] = fakeItem;
    } else {
        let randomPos;
        do {
            randomPos = Math.floor(Math.random() * 12);
        } while (newGridItems[randomPos] !== null);
        newGridItems[randomPos] = fakeItem;
    }

    remainingItems.sort(() => Math.random() - 0.5);

    let remainingItemIndex = 0;
    for (let i = 0; i < 12; i++) {
        if (newGridItems[i] === null) {
            newGridItems[i] = remainingItems[remainingItemIndex++];
        }
    }
    
    createGrid(newGridItems);
}

// Drag and Drop functionality
let draggedItem = null;
gridContainer.addEventListener('dragstart', (e) => { if (e.target.classList.contains('grid-item')) { draggedItem = e.target; setTimeout(() => { e.target.classList.add('dragging'); }, 0); } });
gridContainer.addEventListener('dragend', (e) => { if (draggedItem) { draggedItem.classList.remove('dragging'); draggedItem = null; } });
gridContainer.addEventListener('dragover', (e) => { e.preventDefault(); });
gridContainer.addEventListener('drop', (e) => { e.preventDefault(); if (e.target.classList.contains('grid-item') && draggedItem) { const dropTarget = e.target; const draggedItemIndex = Array.from(gridContainer.children).indexOf(draggedItem); const dropTargetIndex = Array.from(gridContainer.children).indexOf(dropTarget); const isDraggedR1 = draggedItem.textContent === 'R1'; const isTargetEdge = edgeIndices.includes(dropTargetIndex); if (isDraggedR1 && !isTargetEdge) { return; } const isTargetR1 = dropTarget.textContent === 'R1'; const isDraggedEdge = edgeIndices.includes(draggedItemIndex); if(isTargetR1 && !isDraggedEdge) { return; } const isDraggedFake = draggedItem.textContent === 'Fake'; const isTargetFourthRow = fourthRowIndices.includes(dropTargetIndex); if (isDraggedFake && isTargetFourthRow) { return; } const isTargetFake = dropTarget.textContent === 'Fake'; const isDraggedFourthRow = fourthRowIndices.includes(draggedItemIndex); if (isTargetFake && isDraggedFourthRow) { return; } const draggedContent = { text: draggedItem.textContent, color: draggedItem.className.split(' ').find(c => c !== 'grid-item' && c !== 'dragging') }; const targetContent = { text: dropTarget.textContent, color: dropTarget.className.split(' ').find(c => c !== 'grid-item') }; draggedItem.textContent = targetContent.text; draggedItem.className = `grid-item ${targetContent.color}`; dropTarget.textContent = draggedContent.text; dropTarget.className = `grid-item ${draggedContent.color}`; } });


function findOptimalPath() {
    const gridItems = Array.from(gridContainer.children);
    const grid = [];
    while (gridItems.length) {
        grid.push(gridItems.splice(0, 3));
    }

    const pathScores = [];

    // Calculate score for each column (each straight path)
    for (let col = 0; col < 3; col++) {
        let currentScore = 0;
        let isFakePath = false;
        let r2Count = 0;

        for (let row = 0; row < 4; row++) {
            const cell = grid[row][col];
            const text = cell.textContent;

            if (text === 'Fake') {
                isFakePath = true;
                break;
            } else if (text === 'R1') {
                currentScore += R1_PENALTIES[row];
            }
            else if (text === 'R2') {
                currentScore += R2_REWARD_FRONT;
                r2Count++;
            }
        }

        // Apply penalty if there are more than 2 R2 blocks
        if (r2Count > 2) {
            currentScore -= R2_OVERLOAD_PENALTY;
        }

        if (isFakePath) {
            pathScores.push(FAKE_PENALTY);
        }
        else {
            pathScores.push(currentScore);
        }
    }

    // Find the best column
    let maxScore = -Infinity;
    let bestCol = -1;
    for (let i = 0; i < pathScores.length; i++) {
        if (pathScores[i] > maxScore) {
            maxScore = pathScores[i];
            bestCol = i;
        }
    }

    // Clear previous highlights
    grid.flat().forEach(cell => {
        cell.classList.remove('optimal-path', 'optimal-r2-path');
    });

    // Highlight the new best path and update score display
    if (bestCol !== -1 && maxScore > FAKE_PENALTY) {
        scoreDisplay.textContent = `Optimal Path Score: ${maxScore}`;
        for (let row = 0; row < 4; row++) {
            const cell = grid[row][bestCol];
            if (cell.textContent === 'R2') {
                cell.classList.add('optimal-r2-path');
            } else {
                cell.classList.add('optimal-path');
            }
        }
    } else {
        scoreDisplay.textContent = 'No valid path found.';
    }
}

function displayCosts() {
    document.getElementById('cost-r2-front').textContent = R2_REWARD_FRONT;
    document.getElementById('cost-r2-side').textContent = R2_REWARD_DIAGONAL;
    document.getElementById('cost-r1-0').textContent = R1_PENALTIES[0];
    document.getElementById('cost-r1-1').textContent = R1_PENALTIES[1];
    document.getElementById('cost-r1-2').textContent = R1_PENALTIES[2];
    document.getElementById('cost-r1-3').textContent = R1_PENALTIES[3];
    document.getElementById('cost-r2-overload').textContent = `-${R2_OVERLOAD_PENALTY}`;
}

// Event Listeners
randomizeBtn.addEventListener('click', () => {
    randomizeGrid();
    // Clear highlights and score on randomize
    const gridItems = Array.from(gridContainer.children);
    gridItems.forEach(cell => {
        cell.classList.remove('optimal-path', 'optimal-r2-path');
    });
    scoreDisplay.textContent = '';
});

findPathBtn.addEventListener('click', findOptimalPath);

// Initial setup
randomizeGrid();
displayCosts();