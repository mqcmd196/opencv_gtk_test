#include <gtk/gtk.h>
#include <epoxy/gl.h>
#include <iostream>

GLuint program;
GLuint vao;

static GLuint create_shader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    return shader;
}

static void on_realize(GtkGLArea *area) {
    gtk_gl_area_make_current(area);
    if (gtk_gl_area_get_error(area) != NULL)
        return;

    gtk_gl_area_set_required_version(area, 3, 2); // Check OpenGL

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

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

static gboolean on_render(GtkGLArea *area, GdkGLContext *context) {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    return TRUE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GTK3 + OpenGL Example");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    GtkWidget *glarea = gtk_gl_area_new();
    gtk_container_add(GTK_CONTAINER(window), glarea);

    g_signal_connect(glarea, "realize", G_CALLBACK(on_realize), NULL);
    g_signal_connect(glarea, "render", G_CALLBACK(on_render), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);

    gtk_main();
    return 0;
}
