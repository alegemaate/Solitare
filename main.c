#include <allegro.h>
#include <stdio.h>

// Number of cards
#define NUMBER_OF_CARDS 13*4

#define S_DRAW 0
#define S_DISCARD 1
#define S_FOUNDATION 2
#define S_TABLEAU 6
#define S_MOUSE 13
#define NUMBER_OF_STACKS 14

// Card sizes
const int CARD_WIDTH = 81;
const int CARD_HEIGHT = 117;

int counter = 0;

// Buffer
BITMAP *buffer;
BITMAP *card_deck_images[NUMBER_OF_CARDS];
BITMAP *card_sheet;
BITMAP *card_back;

// Functions
void split_deck( BITMAP *new_sheet, int width, int height, BITMAP *card_array[NUMBER_OF_CARDS]);

// Suits
enum {
  hearts, diamonds, clubs, spades
};

// Faces
enum {
  Ace, Two, Three, Four, Five, Six, Seven, Eight, Nine, Ten, Jack, Queen, King
};

// Boolean
typedef enum {
  false = 0, true
} bool;

// Cards
typedef struct card{
  int value;
  int suit;

  bool dealt;

  BITMAP *image[2];

  struct card *card_above;

  int x;
  int y;
} card;

// Stacks
typedef struct{
  card *bottom_card;

  int offset_x;
  int offset_y;

  int x;
  int y;
} stack;

// Stack defs
stack stacks[NUMBER_OF_STACKS];

// Deck
card cards[52];

// Random
int random( int min, int max){
  // Gen
  int random_number = rand()%(max-min) + min;
  return random_number;
}

// Init
void init(){
  // Init allegro
  allegro_init();
  install_mouse();
  install_keyboard();
  install_timer();

  // Graphics
  set_color_depth( 32);
  set_gfx_mode(GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0);

  // Random seed
	srand( time(NULL));

	// Buffer
	buffer = create_bitmap(640, 480);

	// Load card sheet
	card_sheet = load_bitmap( "cards.bmp", NULL);

	// Split the deck
	split_deck( card_sheet, CARD_WIDTH, CARD_HEIGHT, card_deck_images);

	// Card back
  card_back = create_bitmap( CARD_WIDTH, CARD_HEIGHT);
  blit( card_sheet, card_back, 0, 4 * CARD_HEIGHT, 0, 0, CARD_WIDTH, CARD_HEIGHT);

	// Make a deck
  // Cards per suit
  for( int i = 0; i < 13; i++){
    // Number of suits
    for( int t = 0; t < 4; t++){
      // Image
      cards[i + (13 * t)].image[1] = card_deck_images[i + (13 * t)];
      cards[i + (13 * t)].image[0] = card_back;
      // Suit/Value
      cards[i + (13 * t)].suit = t;
      cards[i + (13 * t)].value = i;
      // Facing
      cards[i + (13 * t)].dealt = false;
      // Next card
      cards[i + (13 * t)].card_above = NULL;
    }
  }

  // Nullify stacks
  for( int i = 0; i < NUMBER_OF_STACKS; i++){
    stacks[i].bottom_card = NULL;

    // Mouse pile
    if( i == S_MOUSE){
      stacks[i].x = 0;
      stacks[i].y = 0;
      stacks[i].offset_x = 0;
      stacks[i].offset_y = 10;
    }
    // Tableau piles
    else if( i >= S_TABLEAU){
      stacks[i].x = (i - S_TABLEAU) * (CARD_WIDTH + 5) + 10;
      stacks[i].y = 160;
      stacks[i].offset_x = 0;
      stacks[i].offset_y = 10;
    }
    // Foundation piles
    else if( i >= S_FOUNDATION){
      stacks[i].x = (i - S_FOUNDATION) * (CARD_WIDTH + 10) + 200;
      stacks[i].y = 20;
      stacks[i].offset_x = 0;
      stacks[i].offset_y = 10;
    }
    // Discard pile
    else if( i == S_DISCARD){
      stacks[i].x = 80;
      stacks[i].y = 20;
      stacks[i].offset_x = 0;
      stacks[i].offset_y = 0;
    }
    // Draw pile
    else if( i == S_DRAW){
      stacks[i].x = 20;
      stacks[i].y = 20;
      stacks[i].offset_x = 3;
      stacks[i].offset_y = 0;
    }
  }
}

// Shuffle array
void shuffle( card *array, int length){
  // Temp array
  card temp_array[length];

  // Clean and Copy
  for( int i = 0; i < length; i++){
    temp_array[i] = array[i];
  }

  // Shuffle
  for( int i = 0; i < length; i++){
    // Regen till found
    int random_num = random( 0, length);
    while( temp_array[random_num].value == -1)
      random_num = (random_num + 1) % length;

    // Copy and erase
    array[i] = temp_array[random_num];
    temp_array[random_num].value = -1;
  }
}

// Split deck
void split_deck( BITMAP *new_sheet, int width, int height, BITMAP *card_array[NUMBER_OF_CARDS]){
  // Blit it up
  // Cards per suit
  for( int i = 0; i < 13; i++){
    // Number of suits
    for( int t = 0; t < 4; t++){
      card_array[i + (13 * t)] = create_bitmap( width, height);
      blit( new_sheet, card_array[i + (13 * t)], i * width, t * height, 0, 0, width, height);
    }
  }
}

// Add card to end of stack
void add_to_stack( card **indexCard, card *endCard){
  // Make sure that it is not itself
  if( *indexCard != endCard){
    // If empty, we found the end of stack
    if( *indexCard == NULL){
      *indexCard = endCard;
      endCard -> card_above = NULL;
    }
    // Find the end
    else
      add_to_stack( &((*indexCard) -> card_above), endCard);
  }
}

// Move card to stack
void move_card( card **movingCard, card **newLocation){
  // Make sure movingCard is not null
  if( *movingCard != NULL){
    // Temporary reference
    card *tempCard = *movingCard;
    // Change card reference
    *movingCard = (*movingCard) -> card_above;
    // Deal 1 card
    add_to_stack( newLocation, tempCard);
  }
}

// Deal
void deal(){
  // Clean stacks
  for( int i = 0; i < NUMBER_OF_STACKS; i++)
    stacks[i].bottom_card = NULL;

  // Dealing stack
  for( int i = 0; i < NUMBER_OF_CARDS; i++){
    // Clear reference
    cards[i].card_above = NULL;
    cards[i].dealt = false;

    // Add to pile
    add_to_stack( &stacks[S_DRAW].bottom_card, &cards[i]);
  }

  // Table
  for( int i = 0; i < 7; i++){
    // Cards per stack
    for( int t = 0; t < i + 1; t++){
      // If top card?
      if( t == i)
        stacks[S_DRAW].bottom_card -> dealt = true;

      // Temporary reference
      move_card( &stacks[S_DRAW].bottom_card, &stacks[S_TABLEAU + i].bottom_card);
    }
  }
  rest( 200);
}


// Update
void update(){
  // Shuffle deck
  if( key[KEY_SPACE]){
    shuffle( cards, NUMBER_OF_CARDS);
    deal();
  }

  // Dragging stack
  stacks[S_MOUSE].x = mouse_x;
  stacks[S_MOUSE].y = mouse_y;

  // Click to drag
  if( mouse_b & 1){
    while( mouse_b & 1){}
    for( int i = 0; i < NUMBER_OF_STACKS; i++){
      card *card_ref = stacks[i].bottom_card;
      card *card_ref_past = NULL;

      while( card_ref != NULL){
        if( card_ref -> dealt == true){
          if( mouse_x > card_ref -> x && mouse_x < card_ref -> x + CARD_WIDTH &&
              mouse_y > card_ref -> y && mouse_y < card_ref -> y + CARD_HEIGHT){
            // Pick up
            if( stacks[S_MOUSE].bottom_card == NULL){
              mouse_x = card_ref -> x;
              mouse_y = card_ref -> y;

              move_card( &card_ref, &stacks[S_MOUSE].bottom_card);

              // Bottom of stack
              if( card_ref_past == NULL)
                stacks[i].bottom_card = NULL;
              // Otherwise
              else
                card_ref_past -> card_above = NULL;
              break;
            }
            // Put down
            else if( cards[i].card_above == NULL || cards[i].card_above != NULL){
              move_card( &stacks[S_MOUSE].bottom_card, &card_ref);
              stacks[S_MOUSE].bottom_card = NULL;
              break;
            }
          }
        }
        card_ref_past = card_ref;
        card_ref = card_ref -> card_above;
      }
    }
  }
  if( mouse_b & 2){
    move_card( &stacks[S_MOUSE].bottom_card, &stacks[S_DRAW].bottom_card);
  }
}

// Draw card stack
void draw_stack( card *start_card, int x, int y, int offset_x, int offset_y){
  if( start_card != NULL){
    draw_sprite( buffer, start_card -> image[ start_card -> dealt], x, y);
    start_card -> x = x;
    start_card -> y = y;
    draw_stack( start_card -> card_above, x + offset_x, y + offset_y, offset_x, offset_y);
  }
  else{
    textprintf( buffer, font, x, y, 0xFFFFFF, "NULL");
  }
}

// Draw
void draw(){
  // Clear buffer to green
  clear_to_color( buffer, 0x008800);

  // Draw deck
  for( int i = 0; i < NUMBER_OF_STACKS; i++)
    draw_stack( stacks[i].bottom_card, stacks[i].x, stacks[i].y, stacks[i].offset_x, stacks[i].offset_y);

  // Cursor
  draw_sprite( buffer, mouse_sprite, mouse_x, mouse_y);

  // Draw buffer
  draw_sprite( screen, buffer, 0, 0);
}

// MAIN
int main(){
  // Init
  init();

  // Loop
  while( !key[KEY_ESC]){
    update();
    draw();
  }

  // Exit
  return 0;
}
END_OF_MAIN()
