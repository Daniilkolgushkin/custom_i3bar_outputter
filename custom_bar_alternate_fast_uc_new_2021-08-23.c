#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>

//#define NUM_OF_CORE 4
#define NUM_OF_CORE 5
#define NUM_OF_SECTOR_CORE 6
#define CORE_COLOR(temp) (temp <= coretemp_sector[0]) ? coretemp_color[0] : ((temp <= coretemp_sector[1]) ? coretemp_color[1] : ( (temp <= coretemp_sector[2]) ? coretemp_color[2] : ((temp <= coretemp_sector[3]) ? coretemp_color[3] : ((temp <= coretemp_sector[4]) ? coretemp_color[4] : (coretemp_color[5])))))
#define NUM_OF_SECTOR_BAT 3 
#define MED(in)\
	in;\
	printf(",");
#define START\
	printf("{\"version\":1}\n[\n[]\n");
#define BEGIN_CYCYLE\
	printf(",[");
#define END_CYCLE\
	puts("]");
/*unsigned char *vol;
unsigned char *lout;
unsigned char *network;
unsigned char *bat;
unsigned char *load;
unsigned char *cpu;
unsigned char *avgtemp;
unsigned char *maxtemp;
unsigned char *time_of_date;
unsigned char *tasks;
unsigned char *uptime_of_date;*/

struct timespec global = {};
struct timespec top = {};
struct timespec iw = {};
struct timespec bat = {};

int days = 0;
int hours = 0;
int minutes = 0;
unsigned char day[4];
//int hours = 0;
//int minutes = 0;

const unsigned char uptime_of_date_format_string[] = "{\"full text\": \"up:%d days, %02d:%02d\", \"name\": \"uptime_of_date\"}";
const unsigned char tasks_format_string[] = "{\"full text\": \"tasks:%d(%.1f% R)\", \"name\": \"tasks\"}";
const unsigned char load_format_string[] = "{\"full text\": \"load:%.2f\", \"name\": \"load\"}";
const unsigned char cpuus_format_string[] = "{\"full text\": \"CPU:%.1f%\", \"name\": \"cpu_usage\"}";
const unsigned char memus_format_string[] = "{\"full text\": \"av RAM:%.0f%\", \"name\": \"mem_usage\"}";
const unsigned char swpus_format_string[] = "{\"full text\": \"av SWP:%.0f%\", \"name\": \"swp_usage\"}";
const unsigned char lout_format_string[] = "{\"full text\": \"lout:%s\", \"color\": \"%s\", \"name\": \"lout\"}";
const unsigned char vol_format_string[] = "{\"full text\": \"vol:%d%%%s\", \"color\": \"%s\", \"name\": \"vol\"}";
const unsigned char mic_format_string[] = "{\"full text\": \"mic:%d\%%s\", \"color\": \"%s\", \"name\": \"mic\"}";
const unsigned char wireless_format_string[] = "{\"full text\": \"WiFi:%s\", \"color\": \"%s\", \"name\": \"wireless\"}";
const unsigned char time_of_date_format_string[] = "{\"full text\": \"%s(%s) %s\", \"name\": \"time_of_date\"}";
const unsigned char avg_coretemp_format_string[] = "{\"full text\": \"avg T:%d\", \"color\": \"%s\", \"name\": \"avg_coretemp\"}";
const unsigned char max_coretemp_format_string[] = "{\"full text\": \"max T:%d\", \"color\": \"%s\", \"name\": \"max_coretemp\"}";
const unsigned char bat_format_string[] = "{\"full text\": \"Bat:%.2f%%%s\", \"color\": \"%s\" \"name\": \"battery\"}";
const unsigned char root_format_string[] = "{\"full text\": \"ROOT:%s\", \"name\": \"root_av\"}";
const unsigned char home_format_string[] = "{\"full text\": \"HOME:%s\", \"name\": \"home_av\"}";

///unsigned char top_parse_awk[] = "top -b -n 1 | awk 'BEGIN {FS=\" |,|:\"}; BEGIN {i = 0}; BEGIN {split(\"7 10 11 5 10 15\", a)}; BEGIN {split(\"0 1\", b)}; BEGIN {split(\"0 1 2\", c)}; {for (k in b) if (i == b[k]) for (j in c) print $a[j + 3 * b[k]]}; {i ++}'";
const unsigned char top_parse_awk_configurable[] = "top -b -n 1 | awk 'BEGIN {i = 0}; %s {i ++}'";
//'BEGIN {FS=\" |,|:\"}; 
const unsigned char conf_awk[] = "{if (i == %d) print %s};";

unsigned char parse_top_string[272];

const unsigned char awk_for_tasks[] = "$2 ORS $4";
const int awk_num_for_tasks = 1;
int total;
int running;

const unsigned char awk_for_cpuus[] = "$2 ORS $4";
const int awk_num_for_cpuus = 2;
float cpu_usage;
float cpu_sys;

const unsigned char awk_for_memus[] = "$4 ORS $6";
const int awk_num_for_memus = 3;
float total_mem;
float free_mem;

const unsigned char awk_for_swpus[] = "$3 ORS $5";
const int awk_num_for_swpus = 4;
float total_swp;
float free_swp;

unsigned char awk_for_uptime_of_date[] = "$5 ORS substr($7, 1, 5)";
int awk_num_for_uptime_of_date = 0;

unsigned char awk_for_load[] = "substr($13, 1, length($13) - 1)";
int awk_num_for_load = 0;
float load;
pthread_mutex_t top_read_write;

const unsigned char parse_iw_name[] = "iwconfig 2> /dev/null | grep wl | cut -d \":\" -f 2 | cut -d \"\\\"\" -f 2 | cut -d \" \" -f 1";
const unsigned char parse_iw_signal[] = "iwconfig 2> /dev/null | grep 'Signal level' | cut -d \"=\" -f 3 | cut -d \" \" -f 1";
const unsigned char parse_iw_bit_rate[] = "iwconfig 2> /dev/null | grep Bit | cut -d \"=\" -f 2 | cut -d \" \" -f 1";
const unsigned char parse_iw_quality[] = "iwconfig 2> /dev/null | awk 'BEGIN {FS = \" +|=+\"}; {if ($2 == \"Bit\") print substr($3, 5)}; {if ($1 == \"Link\") print substring($4, 6)}'"; 
const unsigned char iw_color[2][7] = {"FF0000", "00FF00"};
unsigned char iw_name[11];
float iw_signal;
float iw_bit_rate;
unsigned char wireless_state[2][44] = {"Disconnected", ""};
pthread_spinlock_t iw_read_write;

const unsigned char parse_lout[] = "xkblayout-state print \"%n\"";
const unsigned char lout_colors[2][7] = {"0000FF", "FF0000"};
//unsigned char *lout;
unsigned char lout[8];

const unsigned char parse_mic[] = "pactl list source | grep 'Source #1' | grep Muted | cut -d \":\" -f 2";
const unsigned char mic_colors[2][7] = {"00FFFF", "999911"};

const unsigned char parse_vol[] = "pactl list sinks | grep 'Volume: front' | awk '{print $5}'";
const unsigned char parse_vol_muted[] = "pactl list sinks | grep Mute | cut -d \" \" -f 2";
const unsigned char vol_colors[2][7] = {"FFFFFF", "999911"};
unsigned char vol;
unsigned char vol_muted_string[] = "(muted)";
pthread_spinlock_t vol_read_write;

const unsigned char parse_time_of_date[] = "date --rfc-3339='seconds' | cut -d \"+\" -f 1";
const unsigned char parse_day_of_week[] = "date -R | cut -d \" \" -f 1 | cut -d \",\" -f 1";
unsigned char date[11];
unsigned char time_of_date[9];

//const unsigned char parse_coretemp[] = "cat /sys/class/hwmon/hwmon[1,4,6,7]/temp1_input"; 
const unsigned char parse_coretemp[] = "cat /sys/devices/platform/coretemp.0/hwmon/*/temp*_input"; 
int coretemp[NUM_OF_CORE];
const int coretemp_sector[NUM_OF_SECTOR_CORE - 1] = {30, 40, 50, 60, 80};
const unsigned char coretemp_color[NUM_OF_SECTOR_CORE][7] = {"00FF00", "9ACD32", "FFFF00", "FFA500", "FF0000", "FF00FF"};

const unsigned char parse_bat[] = "cat /sys/class/power_supply/BAT0/energy_now /sys/class/power_supply/BAT0/energy_full /sys/class/power_supply/BAT0/power_now /sys/class/power_supply/BAT0/status";
const unsigned char safety_bat[] = "cat /home/daniil/safety_bat_mode";
int bat_now;
int bat_full;
int power_now;
unsigned char state[25];
const unsigned char bat_colors[2 + NUM_OF_SECTOR_BAT][7] = {"999911", "00FFFF", "00FF00", "FFFF00", "FF0000"};
const unsigned char bat_sectors[NUM_OF_SECTOR_BAT - 1][2] = {{8, 15}, {15, 30}};
//pthread_mutex_t bat_read_write;

const unsigned char parse_disk[] = "df -h / /home/daniil | awk '{if ($1 != \"Filesystem\") print $4}'";
unsigned char av_root[5];
unsigned char av_home[5];
//pthread_spinlock_t disk_read_write;


/*void *parse_top() {
	while (1) {
	FILE* topfile = popen("top -b -n 1 | awk 'BEGIN {i = 0}; {if (i == 0) if ($8 == \"minutes\") print \"0\" $7; else print $5 ORS substr($7, 1, 2) ORS substr($7, 4, 2)};{if (i == 0) if ($11 == \"average:\") print substr($13, 1, length($13) - 1); else print substr($14, 1, length($14) - 1)};{if (i == 1) print $2 ORS $4};{if (i == 2) print $2 ORS $4};{if (i == 3) print $4 ORS $6};{if (i == 4) print $3 ORS $5}; {i ++}'", "r");
	pthread_mutex_lock(&top_read_write);
//	fscanf(topfile, "%d", total);
	fscanf(topfile, "%d%d%d%f%d%d%f%f%f%f%f%f", &days, &hours, &minutes, &load, &total, &running, &cpu_usage, &cpu_sys, &total_mem, &free_mem, &total_swp, &free_swp);
	//top_switcher = (top_switcher + 1) % 2;
	pthread_mutex_unlock(&top_read_write);
	pclose(topfile);
	//usleep(32949);
	//usleep(230543);
	//usleep(2305431);
//	testtime.tv_nsec = 2305431000;
		nanosleep(&top, NULL);
	//230543
	//691629
	}
}*/

/*void *parse_iw() {
	while (1) {
	FILE* iw_name_pipe = popen(parse_iw_name, "r");
		pthread_spin_lock(&iw_read_write);
		fscanf(iw_name_pipe, "%s", iw_name);
	
		pclose(iw_name_pipe);
		if (strcmp(iw_name, "off/any")) {
			FILE* iw_signal_pipe = popen(parse_iw_signal, "r");
			fscanf(iw_signal_pipe, "%f", &iw_signal);
			pclose(iw_signal_pipe);
			FILE* iw_bit_rate_pipe = popen(parse_iw_bit_rate, "r");
			fscanf(iw_bit_rate_pipe, "%f", &iw_bit_rate);
			pclose(iw_bit_rate_pipe);
			sprintf(wireless_state[1], "Connected(%s (%.0f dBm / %.0f Mb/s)", iw_name, iw_signal, iw_bit_rate);
		}
	 	pthread_spin_unlock(&iw_read_write);

		//usleep(32623);
	//testtime.tv_nsec = 358853000;
		nanosleep(&iw, NULL);
		//usleep(358853);
		//358853
	}
}*/

void parse_lout_function() {
	FILE* lout_pipe = popen(parse_lout, "r");
	fscanf(lout_pipe, "%s", lout);
	pclose(lout_pipe);
}

void parse_time_of_date_function() {
	FILE* time_of_date_pipe = popen(parse_time_of_date, "r");
	fscanf(time_of_date_pipe, "%s %s", date, time_of_date);
	pclose(time_of_date_pipe);
	FILE* day_of_week_pipe = popen(parse_day_of_week, "r");
	fscanf(day_of_week_pipe, "%s", day);
	pclose(day_of_week_pipe);
}

void parse_coretemp_function() {
	FILE* coretemp_pipe = popen(parse_coretemp, "r");
	for (unsigned char i = 0; i < NUM_OF_CORE; i ++)
		fscanf(coretemp_pipe, "%d", &coretemp[i]);
	pclose(coretemp_pipe);
}

void parse_vol_function() {
//	while(1) {
	FILE* vol_pipe = popen(parse_vol, "r");
//	pthread_spin_lock(&vol_read_write);
	fscanf(vol_pipe, "%d", &vol);
	pclose(vol_pipe);
	FILE* vol_muted_pipe = popen(parse_vol_muted, "r");
	fscanf(vol_muted_pipe, "%s", vol_muted_string);
	if (!strcmp(vol_muted_string, "yes"))
		strcpy(vol_muted_string, "(muted)");
	else
		vol_muted_string[0] = '\0';
//	pthread_spin_unlock(&vol_read_write);
	pclose(vol_muted_pipe);
/*	nanosleep(30613);
	}*/
}


/*void *parse_bat_function() {
	while (1) {
	FILE* bat_pipe = popen(parse_bat, "r");
	pthread_mutex_lock(&bat_read_write);
	fscanf(bat_pipe, "%d\n%d\n%d\n%s", &bat_now, &bat_full, &power_now, state);
	pclose(bat_pipe);
//	bat_pipe = popen(safety_bat, "r");
//	fscanf(bat_pipe, "%d", state[23]);
//	pclose(bat_pipe);
	unsigned char minutes;
	unsigned char hours;
	if(power_now != 0) {
		if (state[0] == 'C') {
			minutes = (unsigned char) ((bat_full - bat_now) * 60 / power_now) % 60;
			hours = (unsigned char) ((bat_full - bat_now) / power_now);
			sprintf(state, ",AC(%d:%02d untill full)", hours, minutes);
			state[24] = 1;
		}
		else {
			minutes = (unsigned int) (bat_now * 60 / power_now) % 60;
			hours = (unsigned char) (bat_now / power_now);
			sprintf(state, "(%d:%02d remaining)", hours, minutes);
			state[24] = (bat_now * 100.0 / bat_full <= bat_sectors[0][0] || (hours == 0 && minutes <= bat_sectors[0][1])) ? 4 : (bat_now * 100.0 / bat_full <= bat_sectors[1][0] || (hours == 0 && minutes <= bat_sectors[1][1])) ? 3 : 2;
		}
	}
	else {
		strcpy(state, ",STATIC");
		state[24] = 0;
	}
	pthread_mutex_unlock(&bat_read_write);
	nanosleep(&bat, NULL);
	}
}*/

/*void *parse_rare() {
	for (unsigned char i = 0; 1; i = (i + 1) % 5) {
	FILE* topfile = popen("top -b -n 1 | awk 'BEGIN {i = 0}; {if (i == 0) if ($8 == \"min,\") print $5 ORS \"0\" ORS $7; else print $5 ORS substr($7, 1, match($7, \":\") - 1) ORS substr($7, match($7, \":\") + 1, 2)};{if (i == 0) if ($11 == \"average:\") print substr($13, 1, length($13) - 1); else print substr($14, 1, length($14) - 1)};{if (i == 1) print $2 ORS $4};{if (i == 2) print $2 ORS $4};{if (i == 3) print $4 ORS $6};{if (i == 4) print $3 ORS $5}; {i ++}'", "r");
	pthread_mutex_lock(&top_read_write);
//	fscanf(topfile, "%d", total);
	fscanf(topfile, "%d%d%d%f%d%d%f%f%f%f%f%f", &days, &hours, &minutes, &load, &total, &running, &cpu_usage, &cpu_sys, &total_mem, &free_mem, &total_swp, &free_swp);
	//top_switcher = (top_switcher + 1) % 2;
	pthread_mutex_unlock(&top_read_write);
	pclose(topfile);
	if (i == 0) {
		FILE* disk_pipe = popen(parse_disk, "r");
		pthread_spin_lock(&disk_read_write);
		fscanf(disk_pipe, "%s %s", av_root, av_home);
		pthread_spin_unlock(&disk_read_write);
		pclose(disk_pipe);
	}
	//usleep(32949);
	//usleep(230543);
	//usleep(2305431);
	//testtime.tv_nsec = 2305431000;
		nanosleep(&top, NULL);
	//230543
	//691629
	}
}*/

void parse_disk_function() {
	FILE* disk_pipe = popen(parse_disk, "r");
	fscanf(disk_pipe, "%s %s", av_root, av_home);
	pclose(disk_pipe);
}

int main() {
	setvbuf(stdout, NULL, _IOFBF, 0);

global.tv_nsec = 250000000;
	START;
	for (int i = 0; 1; i = (i + 1) % 45) {
	BEGIN_CYCYLE;
	printf(",[{\"name\":\"test\", \"full_text\":\"test text\" }]");
	fflush(stdout);
//	parse_top();
	
	if (!(i % 9)) {
	
		FILE* topfile = popen("top -b -n 1 | awk 'BEGIN {i = 0}; {if (i == 0) if ($8 == \"min,\") print $5 ORS \"0\" ORS $7; else print $5 ORS substr($7, 1, match($7, \":\") - 1) ORS substr($7, match($7, \":\") + 1, 2)};{if (i == 0) if ($11 == \"average:\") print substr($13, 1, length($13) - 1); else print substr($14, 1, length($14) - 1)};{if (i == 1) print $2 ORS $4};{if (i == 2) print $2 ORS $4};{if (i == 3) print $4 ORS $6};{if (i == 4) print $3 ORS $5}; {i ++}'", "r");
		fscanf(topfile, "%d%d%d%f%d%d%f%f%f%f%f%f", &days, &hours, &minutes, &load, &total, &running, &cpu_usage, &cpu_sys, &total_mem, &free_mem, &total_swp, &free_swp);
		pclose(topfile);
	}

	if (!(i % 45)) {
		FILE* disk_pipe = popen(parse_disk, "r");
		fscanf(disk_pipe, "%s %s", av_root, av_home);
		pclose(disk_pipe);
	}

//	parse_iw();

	FILE* iw_name_pipe = popen(parse_iw_name, "r");
	fscanf(iw_name_pipe, "%s", iw_name);
	
	pclose(iw_name_pipe);
	if (strcmp(iw_name, "off/any")) {
		FILE* iw_signal_pipe = popen(parse_iw_signal, "r");
		fscanf(iw_signal_pipe, "%f", &iw_signal);
		pclose(iw_signal_pipe);
		FILE* iw_bit_rate_pipe = popen(parse_iw_bit_rate, "r");
		fscanf(iw_bit_rate_pipe, "%f", &iw_bit_rate);
		pclose(iw_bit_rate_pipe);
		sprintf(wireless_state[1], "Connected(%s (%.0f dBm / %.0f Mb/s)", iw_name, iw_signal, iw_bit_rate);
	}

//	parse_bat()
	
	FILE* bat_pipe = popen(parse_bat, "r");
	fscanf(bat_pipe, "%d\n%d\n%d\n%s", &bat_now, &bat_full, &power_now, state);
	pclose(bat_pipe);
	unsigned char minutes;
	unsigned char hours;
	if(power_now != 0) {
		if (state[0] == 'C') {
			minutes = (unsigned char) ((bat_full - bat_now) * 60 / power_now) % 60;
			hours = (unsigned char) ((bat_full - bat_now) / power_now);
			sprintf(state, ",AC(%d:%02d untill full)", hours, minutes);
			state[24] = 1;
		}
		else {
			minutes = (unsigned int) (bat_now * 60 / power_now) % 60;
			hours = (unsigned char) (bat_now / power_now);
			sprintf(state, "(%d:%02d remaining)", hours, minutes);
			state[24] = (bat_now * 100.0 / bat_full <= bat_sectors[0][0] || (hours == 0 && minutes <= bat_sectors[0][1])) ? 4 : (bat_now * 100.0 / bat_full <= bat_sectors[1][0] || (hours == 0 && minutes <= bat_sectors[1][1])) ? 3 : 2;
		}
	}
	else {
		strcpy(state, ",STATIC");
		state[24] = 0;
	}
//	parse_lout_function();
	
	FILE* lout_pipe = popen(parse_lout, "r");
	fscanf(lout_pipe, "%s", lout);
	fread(lout, 1, 8, lout_pipe);
	pclose(lout_pipe);
	
//	end fo parse lout
	
	parse_time_of_date_function();
	parse_coretemp_function();
	
//	parse_vol_function();

	FILE* vol_pipe = popen(parse_vol, "r");
	fscanf(vol_pipe, "%d", &vol);
	pclose(vol_pipe);
	FILE* vol_muted_pipe = popen(parse_vol_muted, "r");
	fscanf(vol_muted_pipe, "%s", vol_muted_string);
	if (!strcmp(vol_muted_string, "yes"))
		strcpy(vol_muted_string, "(muted)");
	else
		vol_muted_string[0] = '\0';
	pclose(vol_muted_pipe);

// end of parsing_vol

//	parse_bat_function();
//	parse_disk_function();
//	pthread_spin_lock(&vol_read_write);
	MED(printf(vol_format_string,  vol, vol_muted_string, vol_colors[(vol_muted_string[0] == '\0') ? 0 : 1]));
//	pthread_spin_unlock(&vol_read_write);
	MED(printf(lout_format_string, lout, lout_colors[(lout[0] == 'E') ? 0 : 1]));
	//pthread_spin_lock(&iw_read_write);
	MED(printf(wireless_format_string, wireless_state[(!strcmp(iw_name, "off/any")) ? 0 : 1], iw_color[(!strcmp(iw_name, "off/any")) ? 0 : 1]));
	//pthread_spin_unlock(&iw_read_write);
	//pthread_spin_lock(&disk_read_write);
	MED(printf(root_format_string, av_root));
	MED(printf(home_format_string, av_home));
	//pthread_spin_unlock(&disk_read_write);
	//pthread_mutex_lock(&top_read_write);
	MED(printf(memus_format_string, free_mem * 100 / total_mem));
	MED(printf(swpus_format_string, free_swp * 100 / total_swp));
	//pthread_mutex_lock(&bat_read_write);
	MED(printf(bat_format_string, (float) bat_now * 100 / bat_full, state, bat_colors[state[24]]));
	//pthread_mutex_unlock(&bat_read_write);
	MED(printf(load_format_string, load));
	MED(printf(tasks_format_string, total, (float) running * 100 / total));
	MED(printf(cpuus_format_string, cpu_usage + cpu_sys));
	//pthread_mutex_unlock(&top_read_write);
	unsigned char avgcoretemp = 0;
	unsigned char maxcoretemp = 0;
	for (unsigned char i = 0; i < NUM_OF_CORE; i ++) {
		maxcoretemp = (maxcoretemp < coretemp[i] / 1000) ? coretemp[i] / 1000 : maxcoretemp;
		avgcoretemp += coretemp[i] / 1000;
	}
	MED(printf(avg_coretemp_format_string, (unsigned char) (avgcoretemp / NUM_OF_CORE), CORE_COLOR(avgcoretemp / NUM_OF_CORE)));
	MED(printf(max_coretemp_format_string, (unsigned char) (maxcoretemp), CORE_COLOR(maxcoretemp)));
	MED(printf(uptime_of_date_format_string, days, hours, minutes));
	printf(time_of_date_format_string, date, day, time_of_date);
	END_CYCLE;
	fflush(stdout);
	if (state[24] == 4 || state[24] == 3)
		system("/home/daniil/BashS/Blink");
	if (state[24] == 2 && state[23] == 1) {
		system("echo \"2\" &> /home/daniil/safety_bat_mode");
		system("systemctl suspend");
	}
//	if ((state[24] == 0 || state[24] == 1) && state[23] == 2)
//		system("echo \"1\" &> /home/daniil/safety_bat_mode");
//	struct timespec testtime = {};
//	testtime.tv_nsec = 250000000;
	nanosleep(&global, NULL);
	}
	
//	pthread_spin_destroy(&vol_read_write);
//	printf("\n%s", uptime_of_date);
	return 0;
}
