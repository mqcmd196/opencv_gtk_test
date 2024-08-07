#include <gtk/gtk.h>
#include <epoxy/gl.h>
#include <iostream>
#include <functional>

class GLWindow {
public:
    GtkWidget *glarea;
    GLWindow(GtkWidget* window) {
        gtk_window_set_title(GTK_WINDOW(window), "GTK3 + OpenGL Example");
        gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
        glarea = gtk_gl_area_new();
        gtk_container_add(GTK_CONTAINER(window), glarea);

        g_signal_connect(glarea, "realize", G_CALLBACK(on_realize), this);
        g_signal_connect(glarea, "render", G_CALLBACK(on_render), this);
        g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    }

    void realize(GtkGLArea *area) {
        gtk_gl_area_make_current(area);
        if (gtk_gl_area_get_error(area) != NULL)
            return;

        if (init_callback) {
            init_callback();
        }
    }

    gboolean render(GtkGLArea *area, GdkGLContext *context) {
        gtk_gl_area_make_current(area);
        if (gtk_gl_area_get_error(area) != NULL)
            return FALSE;

        if (render_callback) {
            render_callback();
        }

        return TRUE;
    }

    void setGlCallback(std::function<void()> callback) {
        render_callback = callback;
    }

    void setInitCallback(std::function<void()> callback) {
        init_callback = callback;
    }

private:
    std::function<void()> render_callback;
    std::function<void()> init_callback;

    static void on_realize(GtkGLArea *area, gpointer user_data) {
        GLWindow *glwindow = static_cast<GLWindow*>(user_data);
        glwindow->realize(area);
    }

    static gboolean on_render(GtkGLArea *area, GdkGLContext *context, gpointer user_data) {
        GLWindow *glwindow = static_cast<GLWindow*>(user_data);
        return glwindow->render(area, context);
    }
};

static GLuint create_shader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    return shader;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    GLWindow glwindow(window);

    GLuint vao, vbo, program;

    glwindow.setInitCallback([&]() {
        glEnable(GL_DEPTH_TEST);
        const char *vertex_shader_source =
                "#version 330 core\n"
                "layout (location = 0) in vec3 position;\n"
                "void main() {\n"
                "   gl_Position = vec4(position, 1.0);\n"
                "}\n";
        const char *fragment_shader_source =
                "#version 330 core\n"
                "out vec4 color;\n"
                "void main() {\n"
                "   color = vec4(1.0, 1.0, 1.0, 1.0); // white\n"
                "}\n";
        GLuint vertex_shader = create_shader(vertex_shader_source, GL_VERTEX_SHADER);
        GLuint fragment_shader = create_shader(fragment_shader_source, GL_FRAGMENT_SHADER);
        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);

        glLinkProgram(program);
        glUseProgram(program);
        GLfloat vertices[] = {
            0.0f,  0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f
        };
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    });

    glwindow.setGlCallback([&]() {
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
    });


    gtk_widget_show(window);
    gtk_widget_show(glwindow.glarea);

    gtk_main();
    return 0;
}
