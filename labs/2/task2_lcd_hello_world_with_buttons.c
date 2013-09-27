#define PushbuttonChanges ((volatile long *) 0x1000005C)
#define LCD_display_ptr ((char *) 0x10003050)


void displayTextLCDWithNewline(char * text_ptr, int newline)
{
	int text_ptr_length = 0;
	
	while ( *(text_ptr) )
	{
		*(LCD_display_ptr + 1) = *(text_ptr);	// write to the LCD data register
		
		++text_ptr;
		text_ptr_length += 1;
	
	}
	
	
	if(newline){
		// Move the cursor to the second line
		*LCD_display_ptr = 0xc0;
	}
	
}

int main()
{ 
	long PBchanges;
    int i;
    int first_button_pressed = 0;

    *PushbuttonChanges = 0; //clear out any changes so far 
    while (1)
    {
		PBchanges = *PushbuttonChanges;
	
	    if(PBchanges){
		
			// The first button is pressed
			if(!first_button_pressed){
				// Clear the display
				*LCD_display_ptr = 0x01;
				// Print "Hello"
				displayTextLCDWithNewline("Hello ",0);
				// The first button has now been pressed
				first_button_pressed = 1;
			} else{ // The second button is pressed
				displayTextLCDWithNewline("World!",1);
				first_button_pressed = 0;
			}
			
			// Wait for result to be actually shown
			for (i=0; i< 10000000;i++){;}
			
			// Reset the changes for the next round
			*PushbuttonChanges = 0;
			
			// Give the user some time to actually press a button
			for (i=0; i< 10000000;i++){;}
			
		}		
    }
}

