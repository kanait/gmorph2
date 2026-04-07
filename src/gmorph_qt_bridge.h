/* C bridge for Qt GUI — keeps X11/Motif macros out of gmorph_qt.cpp */

#ifndef GMORPH_QT_BRIDGE_H
#define GMORPH_QT_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

void gmorph_qt_bridge_set_screen_geom(int screen_idx, int w, int h);
void gmorph_qt_bridge_init_gl_screen(int screen_idx);
void gqt_set_file_context(int filed, int opend);
void gqt_screen_apply_wheel_zoom(int screen_idx, int angle_delta_y);
void gqt_set_qt_in_paint_gl(int on);

struct _screenatr;
void gqt_copy_3d_camera(struct _screenatr *dst, const struct _screenatr *src);

int *gqt_ptr_dis3d_wire(void);
int *gqt_ptr_dis3d_shading(void);
int *gqt_ptr_dis3d_cpoint(void);
int *gqt_ptr_dis3d_cmesh(void);
int *gqt_ptr_dis3d_loop(void);
int *gqt_ptr_dis3d_group(void);
int *gqt_ptr_dis3d_spath(void);
int *gqt_ptr_dis3d_hmap(void);
int *gqt_ptr_dis3d_coaxis(void);
int *gqt_ptr_ext_display(void);
int *gqt_ptr_smooth_shading(void);
int *gqt_ptr_mdiv(void);
int *gqt_ptr_qt_sync_views(void);
int *gqt_ptr_edit_type(void);
void gqt_shortest_paths_create(void);
void gqt_shortest_paths_clear(void);

#ifdef __cplusplus
}
#endif

#endif
