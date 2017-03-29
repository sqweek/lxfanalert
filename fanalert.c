#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <lxpanel/plugin.h>

#define UNUSED(var) ((void)var)

static char* fan_input = "/proc/acpi/ibm/fan";

static int
read_file(char* input_file, char *buf, int n)
{
	int fd, c = 0, r = 0;

	n -= 1; // leave space for NUL terminator

	fd = open(input_file, O_RDONLY);
	while(c < n) {
		r = read(fd, buf+c, n-c);
		if (r == 0) {
			break; // EOF
		} else if (r > 0) {
			c += r;
		} else if (r < 0 && errno != EINTR) {
			goto done;
		}
	}
	buf[c] = '\0';
	r = c;

done:
	close(fd);
	return r;
}

typedef struct {
	gboolean stopped;
	GtkImage* img;
} lxtemp_state;

static void
feedback(lxtemp_state* state, char* icon_name, char* tooltip)
{
	gtk_image_set_from_icon_name(state->img, icon_name, GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_widget_set_tooltip_text(GTK_WIDGET(state->img), tooltip);
}

static gboolean
update(gpointer data)
{
	lxtemp_state* state = (lxtemp_state*)data;
	if (state->stopped) {
		g_free(state);
		return FALSE;
	}

	char buf[256];
	if (read_file(fan_input, buf, sizeof(buf)) > 0) {
		char *line, *save;
		gboolean broke = FALSE;
		for (line = strtok_r(buf, "\n", &save); line != NULL; line = strtok_r(NULL, "\n", &save)) {
			if (strstr(line, "speed:") && strstr(line, "65535")) {
				broke = TRUE; // fan is not working D:
			}
		}
		if (broke) {
			system("notify-send -i dialog-error 'Fan not working' 'Fan is inactive! System will overheat if stressed. Save work and restart at earliest opportunity.'");
			feedback(state, "dialog-error", "Fan is inactive!");
		} else {
			feedback(state, "dialog-information", "Fan operating normally");
		}
	} else {
		sprintf(buf, "code %d (%s)", errno, strerror(errno));
		feedback(state, "dialog-question", buf);
	}
	return TRUE;
}

static void
lxtemp_del(gpointer data) {
	lxtemp_state* state = (lxtemp_state*)data;
	state->stopped = TRUE; // data will be freed on next timer invocation
}

static GtkWidget*
lxtemp_new(LXPanel* panel, config_setting_t* settings)
{
	UNUSED(settings);

	GtkWidget* box = gtk_event_box_new();
	gtk_widget_set_has_window(box, FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(box), 1);

	GtkWidget* image = gtk_image_new();
	gtk_widget_set_style(image, panel_get_defstyle(panel));

	lxtemp_state* state = g_new0(lxtemp_state, 1);
	state->img = (GtkImage*)image;
	update(state);

	gtk_widget_show(image);

	gtk_container_add(GTK_CONTAINER(box), image);
	gtk_widget_set_size_request(box, 26, 26);
	gtk_widget_show(box);

	g_timeout_add_seconds(15, update, state);
	lxpanel_plugin_set_data(box, state, lxtemp_del);
	return box;
}

FM_DEFINE_MODULE(lxpanel_gtk, temperature)

LXPanelPluginInit fm_module_init_lxpanel_gtk = {
	.name = "Fan watchdog",
	.description = "Check fan is operational",

	.new_instance = lxtemp_new,

	.one_per_system = TRUE,
	.expand_available = FALSE,
};
