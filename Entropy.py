from os import path as os_path
import sys
from math import isclose as math_isclose
from enum import Enum
from tkinter import PhotoImage

from matplotlib import patches

try:
    from configparser import ConfigParser
except ImportError:
    from ConfigParser import ConfigParser

from re import sub as re_sub
from operator import itemgetter
import numpy as np
from matplotlib.style import use as mpl_style_use
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, Button, RadioButtons

import entropy_caller


def my_path(path_name):
    """Return the appropriate path for data files based on execution context"""
    if getattr(sys, 'frozen', False):
        # running in a bundle
        return(os_path.join(sys._MEIPASS, path_name))
    else:
        # running live
        return path_name


class Flag(object):

    def __init__(self, start_val=False):
        self.val = start_val

    def lift(self):
        self.val = True

    def drop(self):
        self.val = False

    def change(self):
        self.val = not self.val

    def __bool__(self):
        return self.val


class SteppableDualSliderController(object):

    def __init__(self):
        self.active = SliderId.NONE

    def set_active(self, sid):
        self.active = sid


class SliderId(Enum):
    NONE = 0
    COMP_SIZE = 1
    SUBCOMP_NUM = 2


class LineLabel(Enum):
    BOTH = 'E+D'
    ENTROPY = 'Entropy'
    DATA = 'Data'


class Config:
    def __init__(self):
        config = ConfigParser()
        config.read('settings.ini')

        main_section = 'MAIN'
        self.results_file_name = config.get(main_section, 'results_file_name')
        self.data_file_name = config.get(main_section, 'data_file_name')
        self.data_file_limit = int(config.get(main_section, 'data_file_limit'))
        self.data_convert_commas = bool(config.get(main_section, 'data_convert_commas'))
        result_placement = config.get(main_section, 'result_placement')
        if result_placement == 'start':
            self.result_placement = 0
        elif result_placement == 'middle':
            self.result_placement = 1
        elif result_placement == 'end':
            self.result_placement = 2

        style_section = 'STYLE'
        self.matplotlib_style = config.get(style_section, 'matplotlib_style')
        self.axis_color = config.get(style_section, 'axis_color')
        self.radio_active_color = config.get(style_section, 'radio_active_color')
        self.entropy_line_colour = config.get(style_section, 'entropy_line_colour')
        self.data_line_colour = config.get(style_section, 'data_line_colour')
        self.button_hover_color = config.get(style_section, 'button_hover_color')
        self.default_width = int(config.get(style_section, 'default_width'))
        self.default_height = int(config.get(style_section, 'default_height'))
        self.screen_dpi = int(config.get(style_section, 'screen_dpi'))


def read_data_to_y(c):
    y = []
    file_in = open(c.data_file_name, 'r')
    for line in file_in.readlines():
        y.append(float(line.strip()))

    return y


def slider_step(event, data_size, active_slider, s_comp_size, ax_comp_size, update_comp, s_subcomp_num, ax_subcomp_num,
                update_subcomp):
    if active_slider.active == SliderId.COMP_SIZE:
        if event.key == 'left':
            value = s_comp_size.valmin if math_isclose(s_comp_size.val,
                                                       s_comp_size.valmin) else s_comp_size.val - s_comp_size.valstep
        elif event.key == 'right':
            value = s_comp_size.valmax if math_isclose(s_comp_size.val,
                                                       s_comp_size.valmax) else s_comp_size.val + s_comp_size.valstep
        else:
            value = s_comp_size.val
        ax_comp_size.clear()
        s_comp_size.val = value
        s_comp_size.__init__(ax_comp_size, 'Compartment Size', 1.0, data_size, valinit=value, valstep=1.0,
                             valfmt='%0.0f')
        update_comp(value, active_slider, s_subcomp_num, ax_subcomp_num)
        s_comp_size.on_changed(lambda val: update_comp(val, active_slider, s_subcomp_num, ax_subcomp_num))
    elif active_slider.active == SliderId.SUBCOMP_NUM:
        if event.key == 'left':
            value = s_subcomp_num.valmin if math_isclose(s_subcomp_num.val,
                                                         s_subcomp_num.valmin) else s_subcomp_num.val - s_subcomp_num.valstep
        elif event.key == 'right':
            value = s_subcomp_num.valmax if math_isclose(s_subcomp_num.val,
                                                         s_subcomp_num.valmax) else s_subcomp_num.val + s_subcomp_num.valstep
        else:
            value = s_subcomp_num.val
        ax_subcomp_num.clear()
        s_subcomp_num.val = value
        s_subcomp_num.__init__(ax_subcomp_num, 'Subcompartment Number', 1.0, s_comp_size.val, valinit=value,
                               valstep=1.0, valfmt='%0.0f')
        update_subcomp(value, active_slider)
        s_subcomp_num.on_changed(lambda val: update_subcomp(val, active_slider))


def update_comp(val, active_slider, s_subcomp_num, ax_subcomp_num):
    active_slider.set_active(SliderId.COMP_SIZE)
    val_init = val if s_subcomp_num.val > val else s_subcomp_num.val
    ax_subcomp_num.clear()
    s_subcomp_num.__init__(ax_subcomp_num, 'Subcompartment Number', 1.0, val, valinit=val_init, valstep=1.0,
                           valfmt='%0.0f')
    s_subcomp_num.on_changed(lambda val: update_subcomp(val, active_slider))


def update_subcomp(val, active_slider):
    active_slider.set_active(SliderId.SUBCOMP_NUM)


def submit(event, c, data_size, s_comp_size, s_subcomp_num, entropy_line, ax, fig):
    entropy_caller.py_calculation(
        c.data_file_name,
        c.results_file_name,
        int(s_comp_size.val),
        int(s_subcomp_num.val),
        int(c.result_placement)
    )
    with open(c.results_file_name, 'r') as f:
        lines = f.readlines()
        y_new = [float(line.split()[0]) if len(line.split()) > 0 else np.NaN for line in lines]
    y_new_pad = [np.NaN] * (data_size - len(y_new))
    y_new.extend(y_new_pad)
    np_y = np.array(y_new)
    entropy_line.set_ydata(np_y)
    cur_ylims = ax.get_ylim()
    cur_xlims = ax.get_xlim()
    ax.relim()
    ax.autoscale()
    ax.set_ylim(bottom=min(0.0, cur_ylims[0]))
    ax.set_xlim(left=min(0.0, cur_xlims[0]))
    fig.canvas.draw_idle()


def restore_settings():
    with open(my_path('default.ini'), 'r') as defaults_file, open('settings.ini', 'w') as settings_file:
        for line in defaults_file:
            settings_file.write(line)


def reset(event, restart):
    restore_settings()
    restart.lift()
    plt.close('all')


def colorfunc(label, entropy_line, data_line, fig):
    if label == LineLabel.BOTH.value:
        entropy_line.set_visible(True)
        data_line.set_visible(True)
    elif label == LineLabel.ENTROPY.value:
        entropy_line.set_visible(True)
        data_line.set_visible(False)
    elif label == LineLabel.DATA.value:
        entropy_line.set_visible(False)
        data_line.set_visible(True)
    fig.canvas.draw_idle()


def local_minima(values, indexes):
    result = {}
    for i in indexes:
        result.setdefault(values[i], []).append((i, values[i]))
    return min(result.items(), key=itemgetter(0))[1]


def update_annot(ind, entropy_line, annotation):
    x, y = entropy_line.get_data()
    annotation.xy = (x[ind["ind"][0]], y[ind["ind"][0]])
    annotation.set_text(str(local_minima(y, ind["ind"])))
    annotation.get_bbox_patch().set_alpha(0.4)


def hover(event, entropy_line, annotation, ax, fig):
    if not entropy_line.get_visible():
        return
    vis = annotation.get_visible()
    if event.inaxes == ax:
        cont, ind = entropy_line.contains(event)
        if cont:
            update_annot(ind, entropy_line, annotation)
            annotation.set_visible(True)
            fig.canvas.draw_idle()
        else:
            if vis:
                annotation.set_visible(False)
                fig.canvas.draw_idle()


def set_icon(filepath='entropy.ppm'):
    thismanager = plt.get_current_fig_manager()
    img = PhotoImage(file=my_path(filepath))
    thismanager.window.tk.call('wm', 'iconphoto', thismanager.window._w, img)


def plotting_logic(c, data_size, restart_flag):
    mpl_style_use(c.matplotlib_style)
    fig, ax = plt.subplots(figsize=(c.default_width / c.screen_dpi, c.default_height / c.screen_dpi), dpi=c.screen_dpi)
    set_icon()
    plt.subplots_adjust(left=0.15, bottom=0.25, right=0.95)
    fig.canvas.set_window_title('Entropy K-S')
    active_slider = SteppableDualSliderController()
    x = np.arange(0, data_size)
    y = [0] * data_size
    entropy_line, = plt.plot(x, y, lw=1, color=c.entropy_line_colour)
    y = read_data_to_y(c)
    data_line, = plt.plot(x, y, lw=1, color=c.data_line_colour)
    ax.set_xlabel('Sample No.')
    ax.set_ylabel('Entropy')
    ax.margins(x=0)

    ax_comp_size = plt.axes([0.15, 0.1, 0.80, 0.03], facecolor=c.axis_color)
    ax_subcomp_num = plt.axes([0.15, 0.15, 0.80, 0.03], facecolor=c.axis_color)
    ax_submit = plt.axes([0.85, 0.025, 0.1, 0.04])
    ax_restore = plt.axes([0.70, 0.025, 0.1, 0.04])
    ax_radio = plt.axes([0.025, 0.5, 0.075, 0.125], facecolor=c.axis_color)

    s_comp_size = Slider(ax_comp_size, 'Compartment Size', 1.0, data_size, valinit=2.0, valstep=1.0, valfmt='%0.0f')
    s_subcomp_num = Slider(ax_subcomp_num, 'Subcompartment Number', 1.0, 2.0, valinit=1.0, valstep=1.0, valfmt='%0.0f')

    submit_button = Button(ax_submit, 'Submit', color=c.axis_color, hovercolor=c.button_hover_color)
    button = Button(ax_restore, 'Restore settings', color=c.axis_color, hovercolor=c.button_hover_color)

    radio = RadioButtons(ax_radio, (LineLabel.BOTH.value, LineLabel.ENTROPY.value, LineLabel.DATA.value), active=0,
                         activecolor=c.radio_active_color)

    s_comp_size.on_changed(lambda val: update_comp(val, active_slider, s_subcomp_num, ax_subcomp_num))
    s_subcomp_num.on_changed(lambda val: update_subcomp(val, active_slider))
    fig.canvas.mpl_connect('key_press_event', lambda event: slider_step(event, data_size, active_slider, s_comp_size,
                                                                        ax_comp_size, update_comp, s_subcomp_num,
                                                                        ax_subcomp_num, update_subcomp))
    submit_button.on_clicked(lambda event: submit(event, c, data_size, s_comp_size, s_subcomp_num, entropy_line, ax,
                                                  fig))
    button.on_clicked(lambda event: reset(event, restart_flag))
    radio.on_clicked(lambda label: colorfunc(label, entropy_line, data_line, fig))

    colorfunc(radio.value_selected, entropy_line, data_line, fig)

    annot = ax.annotate("", xy=(0, 0), xytext=(0, -30), textcoords="offset points",
                        bbox=dict(boxstyle="round", fc="w"),
                        arrowprops=dict(arrowstyle="->"), ha='center')
    annot.set_visible(False)

    fig.canvas.mpl_connect("motion_notify_event", lambda event: hover(event, entropy_line, annot, ax, fig))

    plt.show(block=True)


def decimal_separator_set(c):
    if c.data_convert_commas:
        with open(c.data_file_name, 'r') as f:
            data = f.read()
            new_data = re_sub(',', '.', data)

        with open(c.data_file_name, 'w') as f:
            f.write(str(new_data))


def text_window(title, text, iconpath):
    # build a rectangle in axes coords
    left, width = .25, .5
    bottom, height = .25, .5
    right = left + width
    top = bottom + height

    fig = plt.figure(figsize=(8, 3))
    fig.canvas.set_window_title(title)
    set_icon(iconpath)
    ax = fig.add_axes([0, 0, 1, 1])

    ax.text(0.5 * (left + right), 0.5 * (bottom + top), text,
            horizontalalignment='center',
            verticalalignment='center',
            fontsize=20, color='red',
            transform=ax.transAxes)

    ax.set_axis_off()
    ax.axis('off')
    plt.show(block=True)


def main():
    restart = Flag(False)

    while True:

        if not os_path.isfile('settings.ini'):
            text_window('WARNING', "Settings file does not exist - creating", 'warning.ppm')
            restore_settings()

        c = Config()

        if not os_path.isfile(c.data_file_name):
            text_window('ERROR', "Data file does not exist!", 'error.ppm')
            break

        decimal_separator_set(c)

        data_size = entropy_caller.py_read_all_from(c.data_file_name, c.data_file_limit)

        if data_size < 0:
            print("Error when reading from data file.")
            return -1

        plotting_logic(c, data_size, restart)

        if not restart:
            break
        else:
            restart.drop()

    return 0


if __name__ == "__main__":
    main()
