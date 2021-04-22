#include <cmath>
#include <random>
#include <algorithm>
#include <numeric>
#include <windows.h>
#include <iostream>
#include <vector>
#include <list>
using namespace std;

// perlin noise class (not my code)
#ifndef PERLINNOISE_H
#define PERLINNOISE_H

class PerlinNoise {
	// The permutation vector
	std::vector<int> p;
public:
	// Initialize with the reference values for the permutation vector
	PerlinNoise();
	// Generate a new permutation vector based on the value of seed
	PerlinNoise(unsigned int seed);
	// Get a noise value, for 2D images z can have any value
	double noise(double x, double y, double z);
private:
	double fade(double t);
	double lerp(double t, double a, double b);
	double grad(int hash, double x, double y, double z);
};

#endif

PerlinNoise::PerlinNoise() {
	
	// Initialize the permutation vector with the reference values
	p = {
		151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
		8,99,37,240,21,10,23,190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
		35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,
		134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
		55,46,245,40,244,102,143,54, 65,25,63,161,1,216,80,73,209,76,132,187,208, 89,
		18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,
		250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
		189,28,42,223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 
		43,172,9,129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,
		97,228,251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,
		107,49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 };
	// Duplicate the permutation vector
	p.insert(p.end(), p.begin(), p.end());
}

// Generate a new permutation vector based on the value of seed
PerlinNoise::PerlinNoise(unsigned int seed) {
	p.resize(256);

	// Fill p with values from 0 to 255
	std::iota(p.begin(), p.end(), 0);

	// Initialize a random engine with seed
	std::default_random_engine engine(seed);

	// Suffle  using the above random engine
	std::shuffle(p.begin(), p.end(), engine);

	// Duplicate the permutation vector
	p.insert(p.end(), p.begin(), p.end());
}

double PerlinNoise::noise(double x, double y, double z) {
	// Find the unit cube that contains the point
	int X = (int) floor(x) & 255;
	int Y = (int) floor(y) & 255;
	int Z = (int) floor(z) & 255;

	// Find relative x, y,z of point in cube
	x -= floor(x);
	y -= floor(y);
	z -= floor(z);

	// Compute fade curves for each of x, y, z
	double u = fade(x);
	double v = fade(y);
	double w = fade(z);

	// Hash coordinates of the 8 cube corners
	int A = p[X] + Y;
	int AA = p[A] + Z;
	int AB = p[A + 1] + Z;
	int B = p[X + 1] + Y;
	int BA = p[B] + Z;
	int BB = p[B + 1] + Z;

	// Add blended results from 8 corners of cube
	double res = lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z), grad(p[BA], x-1, y, z)), lerp(u, grad(p[AB], x, y-1, z), grad(p[BB], x-1, y-1, z))),	lerp(v, lerp(u, grad(p[AA+1], x, y, z-1), grad(p[BA+1], x-1, y, z-1)), lerp(u, grad(p[AB+1], x, y-1, z-1),	grad(p[BB+1], x-1, y-1, z-1))));
	return (res + 1.0)/2.0;
}

double PerlinNoise::fade(double t) { 
	return t * t * t * (t * (t * 6 - 15) + 10);
}

double PerlinNoise::lerp(double t, double a, double b) { 
	return a + t * (b - a); 
}

double PerlinNoise::grad(int hash, double x, double y, double z) {
	int h = hash & 15;
	// Convert lower 4 bits of hash into 12 gradient directions
	double u = h < 8 ? x : y,
		   v = h < 4 ? y : h == 12 || h == 14 ? x : z;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

// chunk management
vector<vector<int>> north; // positive portion of the world
vector<vector<int>> south; // negative portion of the world
vector<int> generatedChunks; // list of chunks already generated

bool chunkExists(int x) {
    for (int i = 0; i < generatedChunks.size(); i++) {
        if (generatedChunks[i] == x) {
            return true;
        }
    }
    return false;
}

void writeChunk(int x, vector<int> chunk) {
    if (x >= 0) {
        int northAddress = x;
        if (northAddress >= north.size()) {
            north.resize(northAddress+1);
        }
        north[northAddress] = chunk;
    }
    else {
        int southAddress = (-x)-1;
        if (southAddress >= south.size()) {
            south.resize(southAddress+1);
        }
        south[southAddress] = chunk;
    }
}

vector<int> readChunk(int x) {
    if (x >= 0) {
        int northAddress = x;
        return north[northAddress];
    }
    else {
        int southAddress = (-x)-1;
        return south[southAddress];
    }
}

bool createChunk(int x) {
    if (chunkExists(x) == false) {
        vector<int> chunk;
        chunk.resize(512);
        writeChunk(x, chunk);
        generatedChunks.push_back(x);

        return true;
    }
    return false;
}

// block management
int readBlock(int x, int y) {
    vector<int> chunk = readChunk(x);
    if (y >= 0 && y < 512) {
        return chunk.at(y);
    }
    else {
        return 0;
    }
}

void writeBlock(int x, int y, int block) {
    if (y > 0) {
        vector<int> chunk = readChunk(x);
        chunk[y] = block;
        writeChunk(x, chunk);
    }
}

// chunk structure queue
vector<vector<int>> northQueue; // positive portion of the world
vector<vector<int>> southQueue; // negative portion of the world
vector<int> generatedQueueChunks; // list of queue chunks already generated

bool queueChunkExists(int x) {
    for (int i = 0; i < generatedQueueChunks.size(); i++) {
        if (generatedQueueChunks[i] == x) {
            return true;
        }
    }
    return false;
}

void writeQueueChunk(int x, vector<int> chunk) {
    if (x >= 0) {
        int northAddress = x;
        if (northAddress >= northQueue.size()) {
            northQueue.resize(northAddress+1);
        }
        northQueue[northAddress] = chunk;
    }
    else {
        int southAddress = (-x)-1;
        if (southAddress >= southQueue.size()) {
            southQueue.resize(southAddress+1);
        }
        southQueue[southAddress] = chunk;
    }
}

vector<int> readQueueChunk(int x) {
    if (x >= 0) {
        int northAddress = x;
        return northQueue[northAddress];
    }
    else {
        int southAddress = (-x)-1;
        return southQueue[southAddress];
    }
}

bool createQueueChunk(int x) {
    if (queueChunkExists(x) == false) {
        vector<int> chunk;
        chunk.resize(512);
        writeQueueChunk(x, chunk);
        generatedQueueChunks.push_back(x);

        return true;
    }
    return false;
}

void queueBlock(int x, int y, int block) {
    if (chunkExists(x)) {
        writeBlock(x, y, block);
    }
    else {
        if (!queueChunkExists(x)) {
            createQueueChunk(x);
        }
        vector<int> chunk = readQueueChunk(x);
        chunk[y] = block;
        writeQueueChunk(x, chunk);
    }
}

// terain generation
unsigned int seed = (unsigned int) 123;
PerlinNoise perlin(seed);

float getBiome(int x) {
    float biome = perlin.noise((float)x/457,1,1);
    return biome;
}

int getHeightmap(int x, float biome) {
    float ridgePerlin = perlin.noise((float)x/7,1,1);
    float basePerlin = perlin.noise((float)x/57,1,1);
    int depth = 50-floor(80*((basePerlin*biome)+(ridgePerlin*biome*0.1)));
    return depth;
}

void generateChunk(int x) {
    if (createChunk(x)) { // new chunk
        // queued structures
        if (queueChunkExists(x)) {
            vector<int> chunk = readQueueChunk(x);
            for (int y = 0; y < 512; y++) {
                if (chunk[y] > 0) {
                    writeBlock(x, y, chunk[y]);
                }
            }
        }
        // generation
        float biome = getBiome(x);
        int depth = getHeightmap(x,biome);
        int lDepth = getHeightmap(x-1,biome);
        int rDepth = getHeightmap(x+1,biome);
        for (int y = depth; y < 512; y++) {
            bool isCave = !(perlin.noise((float)x/33,(float)y/7,1) > 0.2+((float)y/2048));
            bool isUnderworld = (y > 477+(15*(perlin.noise((float)x/5.3,1,1))));
            if (!isCave && !isUnderworld) {
                writeBlock(x, y, 1);
                // ore
                if ((perlin.noise((float)x/33,(float)y/7,1) < (0.3+((float)y/2096)))) {
                    if (rand() % 100 == 0) {
                        writeBlock(x, y, 10);
                    }
                    if (y > 200 && (rand() % 600 == 0)) {
                        writeBlock(x, y, 11);
                    }
                    if (y > 350 && (rand() % 2000 == 0)) {
                        writeBlock(x, y, 12);
                    }
                }
            }
            if (isCave) {
                // caves
                if (rand() % 200 == 0) {
                    writeBlock(x, y, 13);
                }
            }
        }

        int grassType = (biome > 0.5) ? 4 : 3;
        writeBlock(x, depth-1, grassType);
        writeBlock(x, lDepth-1, grassType);
        writeBlock(x, rDepth-1, grassType);
        for (int y = 480; y < 512; y++) {
            if (y > 490+(10*(perlin.noise((float)x/5.7,1,1)))) {
                writeBlock(x, y, 2);
            }
            else if (y > 494) {
                writeBlock(x, y, 6);
            }
        }
        int groundHeight = depth;
        if (lDepth < groundHeight) {
            groundHeight = lDepth;
        }
        if (rDepth < groundHeight) {
            groundHeight = rDepth;
        }

        // trees
        if (x % 4 == 0 && rand() % 5 == 0) {
            for (int i = 0; i < 4; i++) {
                queueBlock(x, groundHeight-i-2, 7);
            }
            queueBlock(x, groundHeight-6, 9);
            queueBlock(x-1, groundHeight-6, 9);
            queueBlock(x+1, groundHeight-6, 9);
            queueBlock(x, groundHeight-7, 9);
        }

        // surface enemies
        if (x % 8 == 0 && rand() % 17 == 0) {
            writeBlock(x, groundHeight - 2, 13);
        }
    }
}

// console functions
bool keyPressed(int keyID) // returns if the given key is currently held
{
    return (GetAsyncKeyState(keyID) == 0) ? false : true;
}
void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void setCursorPos(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// blocks
class block
{
    public:
        string name; // name of this block
        string text; // character that this block is drawn with
        int color; // color of this block's character
        bool tangible; // player collides with block
        int tool; // what tool can mine this block
        int drop; // item to drop when this block is broken
        int strength; // how long it takes to destroy this block

        block(string sName, string sText, int iColor, bool bTangible, int iTool, int iDrop, int iStrength) { // constructor
            name = sName;
            text = sText;
            color = iColor;
            tangible = bTangible;
            tool = iTool;
            drop = iDrop;
            strength = iStrength;
        };
};

block blocks[] {
    block("air", " ", 7, false, 256, 0, 0),
    block("stone", "/", 8, true, 8, 1, 8),
    block("bedrock", "^", 8, true, 256, 0, 0),
    block("grass", "#", 2, true, 1, 2, 2),
    block("snow", "#", 7, true, 1, 3, 2),
    block("water", "-", 9, false, 256, 0, 0),
    block("lava", "-", 12, false, 256, 0, 0),
    block("log", "|", 14, false, 0, 4, 0),
    block("wood", "%", 14, true, 0, 4, 2),
    block("leaves", "%", 10, true, 0, 4, 0),
    block("iron", "%", 14, true, 8, 9, 10),
    block("diamond", "%", 11, true, 9, 10, 10),
    block("azimuth", "%", 13, true, 10, 11, 20),
    block("monster", "&", 12, true, 8, 11, 12),
};

// inventory
class item
{
    public:
        string name; // name of this item
        int color; // display color of this item
        int type; // what sort of item this is (0 = nothing, 1 = mine tool, 2 = placeable, 3 = armor, 4 = consumable)
        int power; // how powerful this item is
        int meta; // metadata for this item

        item(string sName, int iColor, int iType, int iPower, int iMeta) { // constructor
            name = sName;
            color = iColor;
            type = iType;
            power = iPower;
            meta = iMeta;
        };
};

item items[] {
    item("Hands", 8, 1, 0, 0),
    item("Stone", 8, 2, 0, 1),
    item("Grass", 2, 2, 0, 3),
    item("Snow", 7, 2, 0, 4),
    item("Wood", 14, 2, 0, 8),
    item("Wooden Pickaxe", 14, 1, 1, 8),
    item("Iron Pickaxe", 15, 1, 3, 9),
    item("Diamond Pickaxe", 11, 1, 6, 10),
    item("Azimuth Pickaxe", 13, 1, 10, 11),
    item("Iron Ingot", 15, 0, 0, 0),
    item("Diamond", 11, 0, 0, 0),
    item("Azimuth", 13, 0, 0, 0),
    item("Iron Armor", 15, 3, 20, 0),
    item("Diamond Armor", 11, 3, 50, 0),
    item("Azimuth Armor", 13, 3, 100, 0),
};

const int inventorySize = 8;
int inventorySlot = 1;

int inventory[inventorySize][2] {};

// crafting
class recipe {
    public:
        int input1Type;
        int input1Count;
        int input2Type;
        int input2Count;
        int output;
        int count; 

        recipe(int i1T, int i1C, int i2T, int i2C, int out, int ct) {
            input1Type = i1T;
            input1Count = i1C;
            input2Type = i2T;
            input2Count = i2C;
            output = out;
            count = ct;
        };
};

recipe recipes[] {
    recipe(4, 5, 0, 0, 5, 1),
    recipe(4, 2, 9, 3, 6, 1),
    recipe(4, 2, 10, 3, 7, 1),
    recipe(4, 2, 11, 3, 8, 1),
    recipe(9, 10, 0, 0, 12, 1),
    recipe(10, 10, 0, 0, 13, 1),
    recipe(11, 10, 0, 0, 14, 1),
};

int recipeCount = 7;
int selectedRecipe = 0;

void giveItem(int itemID, int count) {
    if (itemID > 0) {
        // check for item stacks of same type
        for (int i = 0; i < inventorySize; i++) {
            if (inventory[i][0] == itemID) {
                inventory[i][0] = itemID;
                inventory[i][1] += count;
                return;
            }
        }
        // fill up empty slot
        for (int i = 0; i < inventorySize; i++) {
            if (inventory[i][1] == 0) {
                inventory[i][0] = itemID;
                inventory[i][1] += count;
                return;
            }
        }
    }
}

bool hasItem(int itemID, int count) {
    for (int i = 0; i < inventorySize; i++) {
        if (inventory[i][0] == itemID && inventory[i][1] >= count) {
            return true;
        }
    }
    return false;
}

void takeItem(int itemID, int count) {
    for (int i = 0; i < inventorySize; i++) {
        if (inventory[i][0] == itemID && inventory[i][1] >= count) {
            inventory[i][1] -= count;
        }
    }
}

// ui functions
void drawAsciiArt(int x, int y, string *asciiArray, int size) {
    for (int i = 0; i < size; i++) {
        setCursorPos(x,y+i);
        cout << asciiArray[i];
    }
}

void drawOutlineBox(int xPos, int yPos, int invWindowSizeX, int invWindowSizeY) {
    setColor(7);
    for (int y = 0; y < invWindowSizeY; y++) {
        setCursorPos(xPos,y+yPos);
        for (int x = 0; x < invWindowSizeX; x++) {
            if ((x == 0 && y == 0) || (x == invWindowSizeX-1 && y == 0) || (x == 0 && y == invWindowSizeY-1) || (x == invWindowSizeX-1 && y == invWindowSizeY-1)) {
                cout << "+";
            }
            else if (x == 0 || x == invWindowSizeX-1) {
                cout << "|";
            }
            else if (y == 0 || y == invWindowSizeY-1) {
                cout << "-";
            }
            else {
                cout << " ";
            }
        }
    }
}

void drawControlHint(int y, string control, string name) {
    setCursorPos(1,y);
    setColor(3);
    cout << control + "\t";
    setColor(7);
    cout << name;
}

void openInventory(int width, int height) {
    bool firstOpen = true;
    bool invCrafting = true;
    while (true) {
        // input
        setCursorPos(0,0);
        bool inputInteractUp    = keyPressed(VK_UP);
        bool inputInteractDown  = keyPressed(VK_DOWN);
        bool inputInteractLeft  = keyPressed(VK_LEFT);
        bool inputInteractRight = keyPressed(VK_RIGHT);
        bool inputInteractEnter = keyPressed(13);
        bool inputInventory = keyPressed(9);
        bool inputDrop = keyPressed(8);

        if (inputInteractDown) {
            inventorySlot++;
            if (inventorySlot >= inventorySize) {
                inventorySlot = 0;
            }
        }
        if (inputInteractUp) {
            inventorySlot--;
            if (inventorySlot < 0) {
                inventorySlot = inventorySize-1;
            }
        }
        if (inputInventory && !firstOpen) {
            break;
        }
        if (inputDrop && !firstOpen) {
            inventory[inventorySlot][1] = 0;
        }
        if (inputInteractLeft) {
            selectedRecipe--;
            if (selectedRecipe < 0) {
                selectedRecipe = recipeCount-1;
            }
        }
        if (inputInteractRight) {
            selectedRecipe++;
            if (selectedRecipe >= recipeCount) {
                selectedRecipe = 0;
            }
        }
        if (inputInteractEnter) {
            if (hasItem(recipes[selectedRecipe].input1Type, recipes[selectedRecipe].input1Count)) {
                bool secondMaterial = (recipes[selectedRecipe].input2Count <= 0);
                if (!secondMaterial) {
                    secondMaterial = (hasItem(recipes[selectedRecipe].input2Type, recipes[selectedRecipe].input2Count));
                }
                if (secondMaterial) {
                    // can craft it
                    takeItem(recipes[selectedRecipe].input1Type, recipes[selectedRecipe].input1Count);
                    if (recipes[selectedRecipe].input2Count) {
                        takeItem(recipes[selectedRecipe].input2Type, recipes[selectedRecipe].input2Count);
                    }
                    giveItem(recipes[selectedRecipe].output, recipes[selectedRecipe].count);
                }
            }
        }
        
        // draw
        if (firstOpen || (inputInteractUp || inputInteractDown || inputInteractLeft || inputInteractRight || inputInteractEnter || inputInventory || inputDrop)) {
            firstOpen = false;
            drawOutlineBox(2,1,25,3+inventorySize);
            setCursorPos(4,2);
            setColor(8);
            cout << "Inventory";
            for (int i = 0; i < inventorySize; i++) {
                setCursorPos(4,3+i);
                setColor(15);
                cout << ((inventorySlot == i) ? "> " : "  ");
                if (inventory[i][1] > 0) {
                    setColor(items[inventory[i][0]].color);
                    cout << items[inventory[i][0]].name;
                    if (inventory[i][1] > 1) {
                        setColor(8);
                        cout << " x"+std::to_string(inventory[i][1]);
                    }
                }
            }

            // crafting
            if (invCrafting) {
                drawOutlineBox(2,height-6,width-4,4);
                setCursorPos(4,height-5);
                setColor(8);
                cout << "Crafting: ";
                for (int i = 0; i < floor(width/20); i++) {
                    int recipeID = selectedRecipe + i;
                    if (recipeID >= 0 && recipeID < recipeCount) {
                        int recipeOutput = recipes[recipeID].output;
                        if (i == 0) {
                            setColor(15);
                            cout << ">> ";
                        }
                        setColor(items[recipeOutput].color);
                        cout << items[recipeOutput].name;
                        if (i == 0) {
                            setColor(15);
                            cout << " <<";
                        }
                        cout << "   ";
                    }
                }
                setCursorPos(4,height-4);
                setColor(items[recipes[selectedRecipe].input1Type].color);
                cout << items[recipes[selectedRecipe].input1Type].name;
                setColor(8);
                cout << " x";
                cout << to_string(recipes[selectedRecipe].input1Count);
                if (recipes[selectedRecipe].input2Count > 0) {
                    setColor(7);
                    cout << " + ";
                    setColor(items[recipes[selectedRecipe].input2Type].color);
                    cout << items[recipes[selectedRecipe].input2Type].name;
                    setColor(8);
                    cout << " x";
                    cout << to_string(recipes[selectedRecipe].input2Count);
                }
                setColor(8);
                cout << " = ";
                setColor(items[recipes[selectedRecipe].output].color);
                cout << items[recipes[selectedRecipe].output].name;
                setColor(8);
                if (recipes[selectedRecipe].count > 1) {
                    cout << " x";
                    cout << to_string(recipes[selectedRecipe].count);
                }
            }
        }

        Sleep(100);
    }
}

// enemies
bool isEnemy(int blockID) {
    if (blockID == 13) {
        return true;
    }
    return false;
}

// game variables
int playerX = 0;
int playerY = 0;

int cameraX = 0;
int cameraY = 0;

bool jump = false;
int jumpState = 0;

int digCounter;
int digTargetX, digTargetY;

int health = 100;
int armor = 0;
int armorLevel = 100;
int armorColor;

int armorRepairTime = 16;
int armorRepairCounter = armorRepairTime;

int tickEnemyDamage = 0;

// draw routine
void redraw(int width, int height) {
    // draw world
    int enemies[][2] {};
    int enemyNumber = 0;
    setCursorPos(0,0);
    for (int y = 0; y < (height-1); y++) {
        for (int x = 0; x < width; x++) {
            int wx = x+cameraX; // world x
            int wy = y+cameraY; // world y
            if (wx == playerX && wy == playerY) {
                // draw player
                setColor(armorColor);
                std::cout << "&";
            }
            else {
                // draw world
                int blocktype = readBlock(wx,wy);
                
                if (isEnemy(blocktype) == true) {
                    enemies[enemyNumber][0] = wx;
                    enemies[enemyNumber][1] = wy;
                    enemyNumber++;
                }

                setColor(blocks[blocktype].color);
                std::cout << blocks[blocktype].text;
            }
        }
    }

    // enemy ai
    if (armorRepairCounter % 2 == 0) { // enemy move tick
        for (int i = 0; i < enemyNumber; i++) {
            bool enemyJump = false;
            int enX = enemies[i][0];
            int enY = enemies[i][1];
            int blocktype = readBlock(enX,enY);
            float enemyDistance = sqrt(abs(playerX - enX) + abs(playerY - enY));

            if (enemyDistance <= 1) {
                // damage player
                tickEnemyDamage += 10;
            }

            if (enemyDistance < 15 && enemyDistance > 1) {
                // in tracking range
                if (enX > playerX) {
                    // move to left
                    if (readBlock(enX-1,enY) == 0) {
                        writeBlock(enX,enY,0);
                        enX--;
                        writeBlock(enX,enY,blocktype);
                    }
                    else if (readBlock(enX-1,enY-1) == 0) {
                        // jump
                        enemyJump = true;
                        writeBlock(enX,enY,0);
                        enY--;
                        writeBlock(enX,enY,blocktype);
                    }
                }
                else if (enX < playerX) {
                    // move to right
                    if (readBlock(enX+1,enY) == 0) {
                        writeBlock(enX,enY,0);
                        enX++;
                        writeBlock(enX,enY,blocktype);
                    }
                    else if (readBlock(enX+1,enY-1) == 0) {
                        // jump
                        enemyJump = true;
                        writeBlock(enX,enY,0);
                        enY--;
                        writeBlock(enX,enY,blocktype);
                    }
                }
            }
            if (!enemyJump) {
                // gravity
                if (readBlock(enX, enY+1) == 0) {
                    writeBlock(enX, enY, 0);
                    writeBlock(enX, enY+1, blocktype);
                }
            }
        }
    }

    // draw hud
    setCursorPos(0,height-1);
    for (int i = 0; i < width; i++) {
        cout << " ";
    }
    setCursorPos(0,height-1);
    setColor(7);
    if (width > 30) {
        cout << "Health ";
    }
    setColor(12);
    cout << to_string(health)+"/100";
    setColor(7);
    if (width > 30) {
        cout << "   Armor ";
    }
    else {
        cout << " ";
    }
    setColor(3);
    cout << to_string(armor)+"/"+to_string(armorLevel);
    if (inventory[inventorySlot][1] > 0) {
        setColor(7);
        string toolReadout = items[inventory[inventorySlot][0]].name;
        setCursorPos(width-toolReadout.size()-2,height-1);
        cout << "[";
        setColor(items[inventory[inventorySlot][0]].color);
        cout << toolReadout;
        setColor(7);
        cout << "]";
    }
}

// game loop
void gameLoop() {
    while (true) {
        // get terminal window size
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        int height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

        // update camera
        if (playerX > (cameraX + width) - (width/4)) {
            cameraX++;
        }
        if (playerX < cameraX + (width/4)) {
            cameraX--;
        }
        if (playerY > (cameraY + height) - (height/3)) {
            cameraY++;
        }
        if (playerY < cameraY + (height/3)) {
            cameraY--;
        }

        // generate world
        for (int x = 0; x < width; x++) {
            generateChunk(x + cameraX);
        }

        // keyboard input
        bool inputMoveUp = keyPressed(87);
        bool inputMoveDown = keyPressed(83);
        bool inputMoveLeft = keyPressed(65);
        bool inputMoveRight = keyPressed(68);

        bool inputInteractUp    = keyPressed(VK_UP);
        bool inputInteractDown  = keyPressed(VK_DOWN);
        bool inputInteractLeft  = keyPressed(VK_LEFT);
        bool inputInteractRight = keyPressed(VK_RIGHT);
        bool inputInteractEnter = keyPressed(13);
        bool inputInventory = keyPressed(9);

        // inventory
        if (inputInventory) {
            openInventory(width, height);
        }

        // movement
        if (inputMoveLeft) {
            if (!blocks[readBlock(playerX-1, playerY)].tangible) {
                playerX--;
            }
        }
        if (inputMoveRight) {
            if (!blocks[readBlock(playerX+1, playerY)].tangible) {
                playerX++;
            }
        }
        if (inputMoveUp && jump) {
            jumpState = 5;
            jump = false;
        }

        // world interaction
        if (inputInteractUp || inputInteractDown || inputInteractLeft || inputInteractRight) {
            // inventory
            int heldItem = 0;
            if (inventory[inventorySlot][1] > 0) {
                heldItem = inventory[inventorySlot][0];
            }

            // detection
            int interactBlockX,interactBlockY,interactBlock;
            if (inputInteractDown) {
                interactBlockX = playerX;
                interactBlockY = playerY+1;
            }
            if (inputInteractUp) {
                interactBlockX = playerX;
                interactBlockY = playerY-1;
            }
            if (inputInteractRight) {
                interactBlockX = playerX+1;
                interactBlockY = playerY;
            }
            if (inputInteractLeft) {
                interactBlockX = playerX-1;
                interactBlockY = playerY;
            }
            interactBlock = readBlock(interactBlockX, interactBlockY);

            // placement
            if (items[heldItem].type == 2 && interactBlock == 0) {
                writeBlock(interactBlockX, interactBlockY, items[heldItem].meta);
                inventory[inventorySlot][1]--;
            }

            // digging
            if (items[heldItem].type == 1) {
                if (digTargetX != interactBlockX || digTargetY != interactBlockY) {
                    digTargetX = interactBlockX;
                    digTargetY = interactBlockY;
                    digCounter = blocks[interactBlock].strength;
                }
                if (items[heldItem].meta >= blocks[interactBlock].tool) {
                    digCounter--;
                    if (items[heldItem].meta >= blocks[interactBlock].tool && items[heldItem].meta - 4 <= blocks[interactBlock].tool) {
                        digCounter -= items[heldItem].power;
                    }
                }
                if (digCounter < 0) {
                    int itemID = blocks[interactBlock].drop;
                    writeBlock(interactBlockX, interactBlockY, 0);
                    giveItem(itemID, 1);
                }
            }
        }

        // game logic
        if (jumpState > 0) {
            if (!blocks[readBlock(playerX, playerY -1)].tangible) {
                jumpState--;
                playerY--;
                if (jumpState > 1) {
                    playerY--;
                }
            }
            else {
                jumpState = 0;
            }
        }
        if (!blocks[readBlock(playerX, playerY + 1)].tangible) {
            playerY++;
            if (playerY > (cameraY + height) - (height/3)) {
                cameraY++;
            }
        }
        else {
            jump = true;
        }

        // armor
        int inventoryMaxArmor = 0;
        armorColor = 7;
        for (int i = 0; i < inventorySize; i++) {
            if (inventory[i][1] > 0) {
                if (items[inventory[i][0]].type == 3) {
                    // item is armor
                    if (items[inventory[i][0]].power > inventoryMaxArmor) {
                        inventoryMaxArmor = items[inventory[i][0]].power;
                        armorColor = items[inventory[i][0]].color;
                    }
                }
            }
        }
        armorLevel = inventoryMaxArmor;

        armorRepairCounter--;
        if (armorRepairCounter < 0) {
            armorRepairCounter = armorRepairTime;
            armor++;
        }
        if (armor > armorLevel) {
            armor = armorLevel;
        }

        // health
        int playerTouchingBlock = readBlock(playerX, playerY);
        int tickDamage = tickEnemyDamage;
        tickEnemyDamage = 0;

        if (playerTouchingBlock == 6) {
            tickDamage += 10;
        }

        armor -= tickDamage;
        if (armor < 0) {
            health += armor;
            armor = 0;
        }

        if (health > 100) {
            health = 100;
        }
        if (health < 0) {
            health = 0;
            // game over
            string gameOverArt[] {".-----.---.-.--------.-----.","|  _  |  _  |        |  -__|","|___  |___._|__|__|__|_____|","|_____| .-----.--.--.-----.----.","        |  _  |  |  |  -__|   _|","        |_____|\\___/|_____|__|"};
            setColor(12);
            drawAsciiArt((width/2)-16,(height/2)-4,gameOverArt,6);
            Sleep(5000);
            break;
        }

        // draw scene
        redraw(width, height);
    }
}

// main program
int main() {    
    // flush inputs
    keyPressed(87);
    keyPressed(83);
    keyPressed(65);
    keyPressed(68);
    keyPressed(VK_UP);
    keyPressed(VK_DOWN);
    keyPressed(VK_LEFT);
    keyPressed(VK_RIGHT);
    keyPressed(9);
    keyPressed(13);

    // main menu
    int menuSelect;
    while (true) {
        // terminal size
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        int height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

        // draw art
        system("cls");
        string gameLogoArt[] = {"              __                  __   __    ",".---.-.-----.|__|.--------.--.--.|  |_|  |--.","|  _  |-- __||  ||        |  |  ||   _|     |","|___._|_____||__||__|__|__|_____||____|__|__|","        .--.--.---.-.|  |  |.-----.--.--.","        |  |  |  _  ||  |  ||  -__|  |  |","         \\___/|___._||__|__||_____|___  |","                                  |_____|"};
        setColor(3);
        drawAsciiArt((width/2)-22,(height/2)-(width/10),gameLogoArt,8);

        // draw ui
        while (true) {
            setColor(15);
            setCursorPos((width/2)-6,(height/2)+(width/10)-4);
            cout << (menuSelect == 0 ? ">>  New Game  <<" : "    New Game    ");
            setCursorPos((width/2)-6,(height/2)+(width/10)-3);
            cout << (menuSelect == 1 ? ">>  Controls  <<" : "    Controls    ");
            setCursorPos((width/2)-6,(height/2)+(width/10)-2);
            cout << (menuSelect == 2 ? ">>   Credit   <<" : "     Credit     ");
            setCursorPos((width/2)-6,(height/2)+(width/10)-1);
            cout << (menuSelect == 3 ? ">>    Quit    <<" : "      Quit      ");
            bool inputInteractUp    = keyPressed(VK_UP);
            bool inputInteractDown  = keyPressed(VK_DOWN);
            bool inputInteractEnter = keyPressed(13);
            if (inputInteractUp) {
                menuSelect--;
                if (menuSelect < 0) {
                    menuSelect = 3;
                }
            }
            if (inputInteractDown) { 
                menuSelect++;
                if (menuSelect > 3) {
                    menuSelect = 0;
                }
            }
            if (inputInteractEnter) {
                system("cls");
                if (menuSelect == 0) {
                    // start game
                    gameLoop();
                }
                else if (menuSelect == 1) {
                    // controls
                    setCursorPos(1,1);
                    setColor(8);
                    cout << "Player Controls";
                    drawControlHint(2,"WASD","\tMove Player");
                    drawControlHint(3,"ARROWS","\tWorld Interaction");
                    drawControlHint(4,"TAB","\tOpen/Close Inventory");

                    setCursorPos(1,6);
                    setColor(8);
                    cout << "Inventory Controls";
                    drawControlHint(7,"UP/DOWN","Switch Slot");
                    drawControlHint(8,"BACKSPACE","Delete Item");
                    drawControlHint(9,"LEFT/RIGHT","Choose Crafting Recipe");
                    drawControlHint(10,"ENTER","\tCraft Selected Recipe");

                    setCursorPos(1,height-2);
                    setColor(8);
                    system("pause");
                }
                else if (menuSelect == 2) {
                    // credits
                    string creditsText[] {
                        "              __                  __   __    ",
                        ".---.-.-----.|__|.--------.--.--.|  |_|  |--.",
                        "|  _  |-- __||  ||        |  |  ||   _|     |",
                        "|___._|_____||__||__|__|__|_____||____|__|__|",
                        "        .--.--.---.-.|  |  |.-----.--.--.",
                        "        |  |  |  _  ||  |  ||  -__|  |  |",
                        "         \\___/|___._||__|__||_____|___  |",
                        "                                  |_____|",
                        "",
                        "         ---==   Credits   ==---",
                        "",
                        "           Perlin Noise Class",
                        "        (for terrain generation)",
                        "             Paul Silisteanu",
                        "",
                        "            Game Programming",
                        "               Reactified",
                        "",
                        "              Play Testing",
                        "               [Redacted]",
                    };
                    int creditsColors[] {
                        3, 3, 3, 3, 3, 3, 3, 3, 0, 8, 0, 7, 8, 3, 0, 7, 3, 0, 7, 3
                    };
                    setCursorPos(0,3);
                    for (int i = 0; i < 20; i++) {
                        setColor(creditsColors[i]);
                        for (int i = 0; i < (width/2)-20; i++) {
                            cout << " ";
                        }
                        cout << creditsText[i] + "\n";
                        Sleep(300);
                    }
                    setCursorPos(0,0);
                    setColor(0);
                    system("pause");
                }
                else if (menuSelect == 3) {
                    // quit
                    return 0;
                }
                break;
            }
            Sleep(200);
        }
    }
}
