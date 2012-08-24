static void fuzzytime();

void fuzzytime() {
	struct tm tm; 
	time_t	time_value = time(0);
	tm = *localtime(&time_value);
	
	memset(time_ret, '\0', 25);
	int hours = tm.tm_hour;
	int minutes = tm.tm_min;	

	// Update day and date at midnight
	if(hours == 0 && minutes == 0) get_day_date();

	switch (minutes)
	{
		case 58: case 59:
			hours = (hours + 1 );
			break;
		case 53: case 54: case 55: case 56: case 57:
			strcat(time_ret, "five to ");
			hours = (hours + 1);
			break;
		case 48: case 49: case 50: case 51: case 52:
			strcat(time_ret, "ten to ");
			hours = (hours + 1);
			break;
		case 43: case 44: case 45: case 46: case 47:
			strcat(time_ret, "quarter to ");
			hours = (hours + 1);
			break;
		case 38: case 39: case 40: case 41: case 42:
			strcat(time_ret, "twenty to ");
			hours = (hours + 1);
			break;
		case 33: case 34: case 35: case 36: case 37:
			strcat(time_ret, "twenty five to ");
			hours = (hours + 1);
			break;
		case 28: case 29: case 30: case 31: case 32:
			strcat(time_ret, "half past ");
			break;
		case 23: case 24: case 25: case 26: case 27:
			strcat(time_ret, "twenty five past ");
			break;
		case 18: case 19: case 20: case 21: case 22:
			strcat(time_ret, "twenty past ");
			break;
		case 13: case 14: case 15: case 16: case 17:
			strcat(time_ret, "quarter past ");
			break;
		case 8: case 9: case 10: case 11: case 12:
			strcat(time_ret, "ten past ");
			break;
		case 3: case 4: case 5: case 6: case 7:
			strcat(time_ret, "five past ");
			break;
	}
	
	switch (hours)
	{
		case 1: case 13:
			strcat(time_ret, "one ");
			break;
		case 2: case 14:
			strcat(time_ret, "two ");
			break;
		case 3: case 15:
			strcat(time_ret, "three ");
			break;
		case 4: case 16:
			strcat(time_ret, "four ");
			break;
		case 5: case 17:
			strcat(time_ret, "five ");
			break;
		case 6: case 18:
			strcat(time_ret, "six ");
			break;
		case 7: case 19:
			strcat(time_ret, "seven ");
			break;
		case 8: case 20:
			strcat(time_ret, "eight ");
			break;
		case 9: case 21:
			strcat(time_ret, "nine ");
			break;
		case 10: case 22:
			strcat(time_ret, "ten ");
			break;
		case 11: case 23:
			strcat(time_ret, "eleven ");
			break;
		case 12: case 0: case 24:
			strcat(time_ret, "twelve ");
			break;
	}
	switch (minutes)
	{
		case 58: case 59: case 0: case 1: case 2:
			strcat(time_ret, "o'clock ");
			break;
	}
	//strcat(time_ret, "\n");
}
