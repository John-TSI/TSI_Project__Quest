#include<iostream>
#include <cmath> // sqrt, pow
#include<iomanip> // setw
#include<limits> // I/O buffer limit
#include<map> // map<>
#include<string>
#include <time.h> // srand, rand



// ------ TO DO ------

// --- MAJOR ---
// - Ensure compass / treasure is never blocked by obstructions (currently unlikely, but possible)
// - implement choice between 'pacman' map and procedural map
// - implement restart

// --- MINOR ---
// - implement more stats / character customisation
//      - e.g. strength --> break through limited number of boulders
// - implement another event type with map symbol '!'
// - modify terminal output so that map doesn't 'bounce'



// ------ PROTOTYPES ------
int critRoll();
bool luckRoll(int);
void runEvent(auto& adventurer);
void showLegend();
double useCompass(int, int, int, int);



// ------ GLOBAL VARIABLES ------
struct adventurer
{
/*  std::string name = "Adventurer";
    int health = 3;
    int strength = 3; */
    int luck = 3;
    int motivation = 4; 
};



int main()
{
system("cls");
srand(time(NULL)); // seed rand()



std::cout << "\n~o8=====================================8o~" << std::endl;
std::cout << "~o8====[ Welcome to TreasureQuest! ]====8o~" << std::endl;
std::cout << "~o8=====================================8o~\n" << std::endl;



// ------ MAP SPECS ------
int map_depth = 12, map_length = 7;
bool creationFailure;
do{
    creationFailure = false;
    std::cout << "Please enter dimension sizes for your 2D game world:" << std::endl;
    std::cin >> map_depth >> map_length;
    std::cout << "\n" << std::endl;
    
    if(std::cin.fail()) // catches non-numeric user input and flushes the I/O stream buffer
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // ignores rest of user input
        std::cerr << "Invalid input, each dimension size must be an integer value." << std::endl;
        creationFailure = true;
    }

    if(!creationFailure)
    {
        if( (map_depth < 2) || (map_length < 2) )
        {
            std::cout << "Your map is too small! The minimum dimension size is 2.\n" << std::endl;
        }
        if((map_depth > 20) || (map_length > 20))
        {
            std::cout << "Your map is quite large! Dimensions of size <= 20 are recommended.\n" << std::endl;
        }
    }
} while( (map_depth < 2) || (map_length < 2) || (map_depth > 20) || (map_length > 20) || creationFailure);
int map_area = map_depth * map_length, min_dim = (map_depth <= map_length) ? map_depth : map_length;



// ------ DYNAMICALLY ALLOCATE MEMORY FOR MAP ------
int** map = new int*[map_depth];  // map is a pointer to a dynamic array of pointers
map[0] = new int[map_area];  // first pointer within map points to a contiguous dynamic array of L*D ints 
for(int i=1; i<map_depth; i++)
{
    map[i] = map[i-1] + map_length;  // each pointer in map[] contains the memory address L locations further along than the previous pointer
}
// -------------------------------------------------



// ------ MAP INITIALISATION ------
enum State {empty, player, compass, treasure, event, obstruction}; // enumerate state of map coords
int symbCounter = 3;

for(int i=0; i<map_depth; i++)
{
    for(int j=0; j<map_length; j++)
    {
        map[i][j] = State::empty;
    }
}


// --- Randomly initialise obstruction locations ---
while(symbCounter <= int(map_area/3))
{
    int obstruction_x = rand()%map_length, obstruction_y = rand()%map_depth;
    while(map[obstruction_y][obstruction_x] != State::empty)
    {
        obstruction_x = rand()%map_length; // reroll coords if spawn location not empty
        obstruction_y = rand()%map_depth; 
    }
    map[obstruction_y][obstruction_x] = State::obstruction;
    symbCounter++;
}


// --- Randomly initialise event locations ---
while(symbCounter <= int(map_area/2))
{
    int event_x = rand()%map_length, event_y = rand()%map_depth;
    while(map[event_y][event_x] != State::empty)
    {
        event_x = rand()%map_length; // reroll coords if spawn location not empty
        event_y = rand()%map_depth; 
    }
    map[event_y][event_x] = State::event;
    symbCounter++;
} 


// --- Randomly initialise player coords ---
int player_init_x = rand()%map_length, player_init_y = rand()%map_depth;
int player_init_coord_y = map_depth-(player_init_y+1); // coordinate is read using bottom-left array element as 'origin' (0,0)
map[player_init_y][player_init_x] = State::player;

// ensure that the player isn't trapped by obstructions
if(min_dim==map_length)
{
    for(int i=1; i<min_dim; i++)
    {
        if(map[player_init_y][(player_init_x + i)%min_dim] == State::obstruction)
        {
            map[player_init_y][(player_init_x + i)%min_dim] = State::empty;
        }
    }
}
else
{
    for(int i=1; i<min_dim; i++)
    {
        if(map[(player_init_y + i)%min_dim][player_init_x] == State::obstruction)
        {
            map[(player_init_y + i)%min_dim][player_init_x] = State::empty;
        }
    }
}


// --- Randomly initialise compass location ---
int compass_x = rand()%map_length, compass_y = rand()%map_depth;
while(map[compass_y][compass_x] != State::empty)
{
    compass_x = rand()%map_length; // reroll coords if spawn location not empty
    compass_y = rand()%map_depth;
}
map[compass_y][compass_x] = State::compass;

// ensure that the compass isn't trapped by obstructions
if(min_dim==map_length)
{
    for(int i=1; i<min_dim; i++)
    {
        if(map[compass_y][(compass_x + i)%min_dim] == State::obstruction)
        {
            map[compass_y][(compass_x + i)%min_dim] = State::empty;
        }
    }
}
else
{
    for(int i=1; i<min_dim; i++)
    {
        if(map[(compass_y + i)%min_dim][compass_x] == State::obstruction)
        {
            map[(compass_y + i)%min_dim][compass_x] = State::empty;
        }
    }
}


// --- Randomly initialise treasure location ---
int treasure_x = rand()%map_length, treasure_y = rand()%map_depth;
while(map[treasure_y][treasure_x] != State::empty)
{
    treasure_x = rand()%map_length; // reroll coords if spawn location not empty
    treasure_y = rand()%map_depth; 
}
map[treasure_y][treasure_x] = State::treasure;

// ensure that the treasure isn't trapped by obstructions
if(min_dim==map_length)
{
    for(int i=1; i<min_dim; i++)
    {
        if(map[treasure_y][(treasure_x + i)%min_dim] == State::obstruction)
        {
            map[treasure_y][(treasure_x + i)%min_dim] = State::empty;
        }
    }
}
else
{
    for(int i=1; i<min_dim; i++)
    {
        if(map[(treasure_y + i)%min_dim][treasure_x] == State::obstruction)
        {
            map[(treasure_y + i)%min_dim][treasure_x] = State::empty;
        }
    }
}

// ------------------------------------------------



// ------ SYMBOL DICTIONARIES ------
std::map<int,char> symbs = {{State::empty,'_'}, {State::player,'A'}, {State::compass,'C'}, 
                            {State::treasure,'T'}, {State::event,'?'}, {State::obstruction,'@'}}; // for revealed map
std::map<int,char> c_symbs = {{State::empty,'_'}, {State::player,'A'}, {State::compass,'C'}, 
                                {State::treasure,'_'}, {State::event,'?'}, {State::obstruction,'@'}}; // map w/ location of compass revealed
std::map<int,char> hid_symbs = {{State::empty,'_'}, {State::player,'A'}, {State::compass,/*'_'*/'?'}, 
                                {State::treasure,'_'}, {State::event,'?'}, {State::obstruction,'@'}}; // hidden map, only player location and events shown



// ------ DISPLAY MAP ------
auto displayMap = [&](bool mapRevealed=false, bool hasCompass=false) // ugly lambda function to print a map
{
    std::cout << "\n";
    std::cout << "~o8";
    for(int j=0; j<map_length; j++)
    {
        std::cout << std::setw(2) << "==" << std::setw(2) << "==" << std::setw(2) << "==";
    }
    std::cout << "=8o~" << std::endl;

    for(int i=0; i<map_depth; i++)
    {
        std::cout << std::setw(2) << "   ";
        for(int j=0; j<map_length; j++)
        {
            if(j == map_length-1)
            {
                if(mapRevealed)
                {
                    std::cout << std::setw(2) << "|_" << std::setw(2) << symbs[map[i][j]] << std::setw(2) << "_" << "|" << std::endl;
                } 
                else if(hasCompass)
                {
                    std::cout << std::setw(2) << "|_" << std::setw(2) << c_symbs[map[i][j]] << std::setw(2) << "_" << "|" << std::endl;
                }
                else
                {
                    std::cout << std::setw(2) << "|_" << std::setw(2) << hid_symbs[map[i][j]] << std::setw(2) << "_" << "|" << std::endl;
                }
            } 
            else
            {
                if(mapRevealed)
                {
                    std::cout << std::setw(2) << "|_" << std::setw(2) << symbs[map[i][j]] << std::setw(2) << "_";
                }
                else if(hasCompass)
                {
                    std::cout << std::setw(2) << "|_" << std::setw(2) << c_symbs[map[i][j]] << std::setw(2) << "_";
                }
                else
                {
                    std::cout << std::setw(2) << "|_" << std::setw(2) << hid_symbs[map[i][j]] << std::setw(2) << "_";
                }
            }  
        }
    }
    std::cout << "\n" << std::endl; 
}; 



// ------ CHARACTER CREATION ------
int luck_input = 0, player_luck = 3, player_motivation = 4;
adventurer pc;
char response = 'z';
bool bad_response = false, bad_luck_input = false;


do{
    std::cout << "Do you want to customise your stats? (Y/N)" << std::endl;
    std::cout << "> ";
    std::cin >> response;

    switch(response)
    {
        case 'Y':
        case 'y':
            std::cout << "\n~o8===[ You have 7 points to distribute between your LUCK and MOTIVATION stats. ]===8o~" << std::endl;
            std::cout << "\nYour LUCK determines how likely you are to stumble across the TREASURE accidentally." << std::endl;
            std::cout << "Your MOTIVATION determines how likely you are to abandon your search.\n" << std::endl;
            do{
                bad_luck_input = false;
                std::cout << "Enter an integer between 0 and 7 (inclusive) to assign to LUCK:" << std::endl;
                std::cout << "> ";
                std::cin >> luck_input;

                if(std::cin.fail()) // catches non-numeric user input and flushes the I/O stream buffer
                {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // ignores rest of user input
                    std::cerr << "\nInvalid input, please input an integer value." << std::endl;
                    bad_luck_input = true;
                }

                if( (luck_input >=0) && (luck_input <=7) && (!bad_luck_input) )
                {
                    player_luck = luck_input;
                    player_motivation = 7 - luck_input;
                    std::cout << "\n~o8====[ Your stats have been assigned: LUCK --> " << luck_input << ", MOTIVATION --> " << player_motivation << " ]====8o~\n\n" << std::endl;
                    bad_luck_input = false;
                }
                else if(!bad_luck_input)
                {
                    std::cout << "\nInvalid input, ensure that 0 <= LUCK <= 7.\n" << std::endl;
                    bad_luck_input = true;
                }
            }while(bad_luck_input);
            bad_response = false;
            break;
        case 'N':
        case 'n':
            std::cout << "\n~o8====[ Your stats have been assigned. ]====8o~\n\n" << std::endl;
            bad_response = false;
            break;
        default:
            std::cout << "\nInvalid input, please try again.\n" << std::endl;
            bad_response = true;
            
    }
} while(bad_response);

pc.luck = player_luck;
pc.motivation = player_motivation;



// ------ INPUT HANDLING ------
char playerChoice = 'z';
std::cout << "~o8======[ The world has been randomly generated. ]======8o~" << std::endl;
std::cout << "You have spawned at coordinates (" << player_init_x << "," << player_init_coord_y << ") on the map below:" << std::endl; 
bool mapRevealed = false, hasCompass = false; 
displayMap(mapRevealed, hasCompass);


int player_x = player_init_x, player_y = player_init_y, player_coord_y = 0;
bool chanceFind = true;

while(playerChoice != 'q')
{
std::cout << "How will you proceed?" << std::endl;
std::cout << "--------------------------------" << std::endl;
std::cout << "Enter 'w' to move North." << std::endl;
std::cout << "Enter 's' to move South." << std::endl;
std::cout << "Enter 'd' to move East." << std::endl;
std::cout << "Enter 'a' to move West." << std::endl;
std::cout << "Enter 'h' to view the map legend." << std::endl; 
std::cout << "Enter 'q' to abandon your quest." << std::endl;
std::cout << "--------------------------------" << std::endl; 
std::cout << "> ";
std::cin >> playerChoice;



// ------ INPUT HANDLING ------
map[player_y][player_x] = State::empty;
bool bad_input = false, obstructed = false, legend = false; 
switch(playerChoice) // treats the map as a torus
{
    case 'w':
        if(player_y > 0){ (map[player_y - 1][player_x] == State::obstruction) ? obstructed = true : player_y -= 1; }
        else{ (map[map_depth - 1][player_x] == State::obstruction) ? obstructed = true : player_y = map_depth - 1; }
        break;
    case 's':
        if(player_y < (map_depth - 1)){ (map[player_y + 1][player_x] == State::obstruction) ? obstructed = true : player_y += 1; }
        else{ (map[0][player_x] == State::obstruction) ? obstructed = true : player_y = 0; }
        break;
    case 'd':
        if(player_x < (map_length - 1)){ (map[player_y][player_x + 1] == State::obstruction) ? obstructed = true : player_x += 1; }
        else{ (map[player_y][0] == State::obstruction) ? obstructed = true : player_x = 0; }
        break;
    case 'a':
        if(player_x > 0){ (map[player_y][player_x - 1] == State::obstruction) ? obstructed = true : player_x -= 1; }
        else{ (map[player_y][map_length - 1] == State::obstruction) ? obstructed = true : player_x = map_length-1; }
        break;
     case 'h':
        showLegend();
        legend = true;
        break; 
    case 'q':
        std::cout << "\n\nFarewell adventurer! Look out for the upcoming sequel:-\n" << std::endl;
        std::cout << "~o8======[ TreasureQuest II : Even Questier ]======8o~\n" << std::endl;
        return 0;
    default:
        std::cout << "\nInvalid input, please stop trying to sabotage your quest.\n" << std::endl;
        bad_input = true;
}


// ------ EVENT HANDLING ------
if(obstructed)
{
    system("cls");
    std::cout << "\nThis direction is obstructed by a large boulder, try another path.";
}


if((player_x == compass_x) && (player_y == compass_y) && (!hasCompass))
{
    map[player_y][player_x] = State::compass;
    system("cls"); 
    std::cout << "\n~o8=============================================8o~" << std::endl;
    std::cout << "~o8===[ You have obtained the MAGIC COMPASS ]===8o~" << std::endl;
    std::cout << "~o8=============================================8o~" << std::endl;
    hasCompass = true; 
}
else if(map[player_y][player_x] == State::event)
{
    map[player_y][player_x] = State::player;
    system("cls"); 
    std::cout << "\n~o8===[ You have triggered an EVENT ]===8o~" << std::endl;
    runEvent(pc);
    if(pc.motivation <= 0)
    {
        std::cout << "\nYour motivation is dwindling, and you doubt the treasure even exists." << std::endl;
        if(!critRoll())
        {
            std::cout << "\n~o8===[ You ABANDON your quest ]===8o~\n\n";
            return 0;
        }
        else
        {
            std::cout << "Nevertheless, you decide to keep wandering." << std::endl;

        }
    }
} else
{
    map[player_y][player_x] = State::player;
    if(!obstructed && !legend && !bad_input){system("cls");} 
} 


if((player_x == treasure_x) && (player_y == treasure_y))
{
    if(hasCompass)
    {
        map[player_y][player_x] = State::treasure;
        system("cls"); 
        std::cout << "\n~o8=======================================================8o~" << std::endl;
        std::cout << "~o8===[ You have found the TREASURE, congratulations! ]===8o~" << std::endl;
        std::cout << "~o8=======================================================8o~\n" << std::endl;
        mapRevealed = true;
        displayMap(mapRevealed, hasCompass);
        return 0;
    }
    else if(luckRoll(pc.luck) && chanceFind)
    {
        map[player_y][player_x] = State::treasure;
        system("cls"); 
        std::cout << "\n~o8============================================================8o~" << std::endl;
        std::cout << "~o8===[ You stumbled across the TREASURE by chance, lucky! ]===8o~" << std::endl;
        std::cout << "~o8============================================================8o~\n" << std::endl;
        mapRevealed = true;
        displayMap(mapRevealed, hasCompass);
        return 0;
    }
    else
    {
        std::cout << "\n~o8===[ The TREASURE must be nearby, if only you had a COMPASS... ]===8o~" << std::endl;
        chanceFind = false;
    } 
}


if(!bad_input){
    player_coord_y = map_depth-(player_y+1);
    if(obstructed || legend)
    {
        std::cout << "\nYou are still at coordinates (" << player_x << "," << player_coord_y << ")." << std::endl;
    }
    else
    {
        std::cout << "\nYou are currently at coordinates (" << player_x << "," << player_coord_y << ") and your map has been updated:" << std::endl; 
    }
    displayMap(mapRevealed, hasCompass);
    if(hasCompass)
    {
        std::cout << "~o8===[ The treasure is " << std::setprecision(2) << useCompass(player_x, player_y, treasure_x, treasure_y) << " unit(s) away from your current location... ]===8o~\n" << std::endl;
    }
}

} // end while



// ------ DEALLOCATE MAP MEMORY ------
delete[] map[0];  
delete[] map;
// -----------------------------------



//std::cout << "The program isn't broken yet." << std::endl;



return 0;
} // end main



// ------ FUNCTION DEFINITIONS ------

// randomly return either 0 or 1, called to determine whether player loses
int critRoll()
{
    return rand()%2;
}


bool luckRoll(int luck)
{
    int thresh = 10, roll = rand()%(8 + 2*luck) + (luck);
    bool win = (roll >= thresh) ? true : false; 
    return win;

}


// randomly execute an event, called when player encounters a map event
void runEvent(auto& pc) // pass object of type struct adventurer by reference
{
    switch(rand()%13)
    {
// --- MOTIVATION MODIFIERS ---
        case 0:
            std::cout << "\nYou find yourself becoming less optimistic about your quest..." << std::endl; 
            pc.motivation--;
            std::cout << "Your MOTIVATION has decreased to " << pc.motivation << "." << std::endl;
            break;
        case 1:
            std::cout << "\nThe promise of treasure rejuvenates you!" << std::endl;
            pc.motivation++;
            std::cout << "Your MOTIVATION has increased to " << pc.motivation << "." << std::endl;
            break;
        case 2:
            std::cout << "\nDid you lock your front door? Perhaps you should go home and check..." << std::endl;
            pc.motivation--;
            std::cout << "Your MOTIVATION has decreased to " << pc.motivation << "." << std::endl;
            break;
        case 3:
            std::cout << "\nThis quest is easy, there aren't even any enemies!" << std::endl;
            pc.motivation++;
            std::cout << "Your MOTIVATION has increased to " << pc.motivation << "." << std::endl;
            break;
        case 4:
            std::cout << "\nYou are convinced you've seen this map square already..." << std::endl;
            pc.motivation--;
            std::cout << "Your MOTIVATION has decreased to " << pc.motivation << "." << std::endl;
            break;
        case 5:
            std::cout << "\nAs a fan of boulders, all of these boulders make you happy!" << std::endl;
            pc.motivation++;
            std::cout << "Your MOTIVATION has increased to " << pc.motivation << "." << std::endl;
            break;
        case 6:
        case 7:
            std::cout << "\nYou fall in a puddle." << std::endl;
            pc.motivation--;
            std::cout << "Your MOTIVATION has decreased to " << pc.motivation << "." << std::endl;
            break;
        case 8:
            std::cout << "\nYour shoes feel a bit tight." << std::endl;
            pc.motivation--;
            std::cout << "Your MOTIVATION has decreased to " << pc.motivation << "." << std::endl;
            break;
       case 9:
            std::cout << "\nYou suddenly realise that you're just a game character." << std::endl;
            pc.motivation = 0;
            std::cout << "Your MOTIVATION has decreased to " << pc.motivation << "." << std::endl;
            break;
// --- LUCK MODIFIERS ---
        case 10:
            std::cout << "\nYou drop and break your trusty portable mirror." << std::endl;
            pc.luck -= 2;
            std::cout << "Your LUCK has decreased to " << pc.luck << "." << std::endl;
            break;
        case 11:
            std::cout << "\nYou have befriended a local leprechaun." << std::endl;
            pc.luck += 2;
            std::cout << "Your LUCK has increased to " << pc.luck << "." << std::endl;
            break;
        case 12:
            std::cout << "\nYou have offended a leprechaun by dropping his favourite mirror." << std::endl;
            pc.luck -= 10;
            std::cout << "Your LUCK has decreased to " << pc.luck << "." << std::endl;
            break;
    }
}


// prints a map legend
void showLegend()
{
    std::cout << "\n\n~o8======[ LEGEND ]======8o~" << std::endl;
    std::cout << "  A .... Adventurer (you)" << std::endl;
    std::cout << "  @ .... Impassable Boulder" << std::endl;
    std::cout << "  ? .... An Event" << std::endl;
    std::cout << "  C .... Compass (revealed on pickup)" << std::endl;
    std::cout << "  T .... Treasure (revealed on pickup)\n" << std::endl;
}


// return Euclidean distance between player and treasure
double useCompass(int p_x, int p_y, int t_x, int t_y)
{
    return sqrt( pow((t_y - p_y), 2) + pow((t_x - p_x), 2)); 
}
