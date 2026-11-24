// Card game scene - Graphics assignment (OpenGL / GLFW / GLM)
// Built on top of the lecture code: PolygonWithTexture (L4/L5), lookAt + perspective (L3),
// glm transformations (L2/L3), stb_image textures (L4/L5).

#include "libs/glad_loader_docs.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader_m.h>
#include "stb_image.h"
#include "tools/Polygon.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace std;
using namespace glm;

// ----------------------------------------------------------------------------
// Settings / scene dimensions (all in world units, Y is up)
// ----------------------------------------------------------------------------
const int SCR_WIDTH = 1280;
const int SCR_HEIGHT = 720;

const float ROOM_HALF = 6.0f;   // room spans x,z in [-6, 6]
const float ROOM_HEIGHT = 6.5f;
const float TABLE_TOP_Y = 1.0f; // y of the table surface

const float CARD_W = 0.85f;
const float CARD_H = 1.2f;
const float CARD_Y = TABLE_TOP_Y + 0.02f; // a hair above the table to avoid z-fighting
const float CARD_SPACING = 1.15f;
const float DECK_Z = 1.575f + CARD_H / 2.0f; // pivot (near edge) of the cards in the deck zone
const float PLAY_Z = 0.0f + CARD_H / 2.0f;   // pivot of the cards in the play zone
const float STAND_ANGLE = radians(78.0f);    // how far a card tilts up when it "stands"

const float CAMERA_TRANSITION_TIME = 0.9f; // seconds to go from state 1 to state 2 (and back)
const float CARD_MOVE_TIME = 0.9f;         // seconds for a card to fly deck -> play zone

// Two fixed camera states
const vec3 CAM1_EYE(0.0f, 8.0f, 0.0f), CAM1_TARGET(0.0f, TABLE_TOP_Y, 0.0f), CAM1_UP(0.0f, 0.0f, -1.0f); // top view
const vec3 CAM2_EYE(0.0f, 4.6f, 5.6f), CAM2_TARGET(0.0f, 1.15f, 0.2f), CAM2_UP(0.0f, 1.0f, 0.0f);       // player view

// ----------------------------------------------------------------------------
// Input state (written by the key callback, read by the main loop)
// ----------------------------------------------------------------------------
float g_standTarget = 0.0f; // 0 = camera 1 (cards flat), 1 = camera 2 (cards standing)
int g_pendingDraws = 0;     // number of times G was pressed and not yet handled

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;
    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, true);
    else if (key == GLFW_KEY_1)
        g_standTarget = 0.0f;
    else if (key == GLFW_KEY_2)
        g_standTarget = 1.0f;
    else if (key == GLFW_KEY_G)
        g_pendingDraws++;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------
GLuint loadTexture(const string &path)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    int width, height, channels;
    // always ask for 4 channels (RGBA) so PNGs with transparency (the torn poster) work
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (data)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
        cout << "Failed to load texture: " << path << endl;
    stbi_image_free(data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return texture;
}

// A textured quad from 4 corners (given in order around the quad).
// uvMax > 1 repeats the texture (GL_REPEAT).
void addQuad(vector<PolygonWithTexture> &out, vec3 a, vec3 b, vec3 c, vec3 d, GLuint texture,
             vec2 uvMax = vec2(1.0f), vec3 tint = vec3(1.0f))
{
    vector<vec3> v = {a, b, c, d};
    vector<vec2> t = {vec2(0, 0), vec2(uvMax.x, 0), vec2(uvMax.x, uvMax.y), vec2(0, uvMax.y)};
    PolygonWithTexture q(v, t, tint);
    q.setTexture(texture);
    out.push_back(q);
}

// An axis aligned box made of 6 quads. frontTexture (optional) is used for the +Z face.
void addBox(vector<PolygonWithTexture> &out, vec3 mn, vec3 mx, GLuint texture, GLuint frontTexture = 0)
{
    GLuint front = frontTexture ? frontTexture : texture;
    addQuad(out, vec3(mn.x, mn.y, mx.z), vec3(mx.x, mn.y, mx.z), vec3(mx.x, mx.y, mx.z), vec3(mn.x, mx.y, mx.z), front); // +Z
    addQuad(out, vec3(mx.x, mn.y, mn.z), vec3(mn.x, mn.y, mn.z), vec3(mn.x, mx.y, mn.z), vec3(mx.x, mx.y, mn.z), texture); // -Z
    addQuad(out, vec3(mx.x, mn.y, mx.z), vec3(mx.x, mn.y, mn.z), vec3(mx.x, mx.y, mn.z), vec3(mx.x, mx.y, mx.z), texture); // +X
    addQuad(out, vec3(mn.x, mn.y, mn.z), vec3(mn.x, mn.y, mx.z), vec3(mn.x, mx.y, mx.z), vec3(mn.x, mx.y, mn.z), texture); // -X
    addQuad(out, vec3(mn.x, mx.y, mx.z), vec3(mx.x, mx.y, mx.z), vec3(mx.x, mx.y, mn.z), vec3(mn.x, mx.y, mn.z), texture); // +Y
    addQuad(out, vec3(mn.x, mn.y, mn.z), vec3(mx.x, mn.y, mn.z), vec3(mx.x, mn.y, mx.z), vec3(mn.x, mn.y, mx.z), texture); // -Y
}

float smooth01(float t)
{
    t = glm::clamp(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// ----------------------------------------------------------------------------
// Card
// ----------------------------------------------------------------------------
enum class CardState { InDeck, Moving, Played };

struct Card
{
    PolygonWithTexture mesh;
    vec3 deckPos;        // pivot position (the card's near edge) while in the deck
    vec3 playPos;        // pivot position once played
    CardState state = CardState::InDeck;
    float progress = 0.0f; // 0..1 while moving

    Card(PolygonWithTexture m, vec3 deck) : mesh(m), deckPos(deck), playPos(deck) {}

    // Current pivot position (interpolated while moving, with a small arc).
    vec3 position() const
    {
        if (state == CardState::InDeck)
            return deckPos;
        if (state == CardState::Played)
            return playPos;
        float e = smooth01(progress);
        vec3 p = mix(deckPos, playPos, e);
        p.y += sin(pi<float>() * e) * 0.9f; // lift the card while it travels
        return p;
    }
};

// ----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // Run from the folder of the exe so "./textures" and "./shaders" are found.
    if (argc > 0)
    {
        error_code ec;
        auto dir = filesystem::canonical(argv[0], ec).parent_path();
        if (!ec)
            filesystem::current_path(dir, ec);
    }

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Card Game", NULL, NULL);
    if (!window)
    {
        cout << "Failed to create window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_DEPTH_TEST);
    stbi_set_flip_vertically_on_load(true);

    Shader shader("./shaders/vs/Card.vs", "./shaders/fs/Card.fs");

    // ---------------- textures ----------------
    GLuint texFloor = loadTexture("./textures/floor.png");
    GLuint texWall = loadTexture("./textures/wallpaper.png");
    GLuint texTable = loadTexture("./textures/table.png");
    GLuint texDarkWood = loadTexture("./textures/darkwood.png");
    GLuint texZone = loadTexture("./textures/zone.png");
    GLuint texPoster = loadTexture("./textures/poster.png");
    GLuint texClock = loadTexture("./textures/clock.png");
    GLuint texAquarium = loadTexture("./textures/aquarium.png");
    GLuint texArcade = loadTexture("./textures/arcade.png");
    vector<GLuint> texCards;
    for (int i = 0; i < 5; i++)
        texCards.push_back(loadTexture("./textures/card" + to_string(i) + ".png"));

    // ---------------- static scene ----------------
    vector<PolygonWithTexture> statics;
    const float R = ROOM_HALF, H = ROOM_HEIGHT;

    // room: wooden floor + 4 wallpapered walls
    addQuad(statics, vec3(-R, 0, R), vec3(R, 0, R), vec3(R, 0, -R), vec3(-R, 0, -R), texFloor, vec2(4, 4));
    addQuad(statics, vec3(-R, 0, -R), vec3(R, 0, -R), vec3(R, H, -R), vec3(-R, H, -R), texWall, vec2(4, 2)); // back
    addQuad(statics, vec3(R, 0, R), vec3(-R, 0, R), vec3(-R, H, R), vec3(R, H, R), texWall, vec2(4, 2));     // front
    addQuad(statics, vec3(-R, 0, R), vec3(-R, 0, -R), vec3(-R, H, -R), vec3(-R, H, R), texWall, vec2(4, 2)); // left
    addQuad(statics, vec3(R, 0, -R), vec3(R, 0, R), vec3(R, H, R), vec3(R, H, -R), texWall, vec2(4, 2));     // right

    // table: top + 4 legs
    addBox(statics, vec3(-3.4f, TABLE_TOP_Y - 0.1f, -2.5f), vec3(3.4f, TABLE_TOP_Y, 2.5f), texTable);
    for (float sx : {-1.0f, 1.0f})
        for (float sz : {-1.0f, 1.0f})
            addBox(statics, vec3(sx * 3.15f - 0.15f, 0.0f, sz * 2.25f - 0.15f), vec3(sx * 3.15f + 0.15f, TABLE_TOP_Y - 0.1f, sz * 2.25f + 0.15f), texDarkWood);

    // three table zones: opponent (empty), play, player's deck
    float zy = TABLE_TOP_Y + 0.003f;
    auto zone = [&](float z0, float z1, vec3 tint) {
        addQuad(statics, vec3(-3.0f, zy, z1), vec3(3.0f, zy, z1), vec3(3.0f, zy, z0), vec3(-3.0f, zy, z0), texZone, vec2(1, 1), tint);
    };
    zone(-2.3f, -0.85f, vec3(1.0f, 0.75f, 0.75f)); // opponent
    zone(-0.7f, 0.7f, vec3(0.8f, 1.0f, 0.85f));    // play area
    zone(0.85f, 2.3f, vec3(1.0f, 0.9f, 0.7f));     // deck

    // poster on the back wall (the texture itself has a torn corner: alpha = 0 there)
    addQuad(statics, vec3(-1.4f, 2.0f, -R + 0.02f), vec3(1.4f, 2.0f, -R + 0.02f), vec3(1.4f, 5.8f, -R + 0.02f), vec3(-1.4f, 5.8f, -R + 0.02f), texPoster);

    // 3 decor pieces (against the back wall): grandfather clock, aquarium, arcade cabinet
    addBox(statics, vec3(-5.1f, 0.0f, -5.9f), vec3(-3.9f, 4.0f, -5.1f), texDarkWood, texClock);
    addBox(statics, vec3(2.0f, 0.0f, -5.9f), vec3(4.0f, 1.1f, -5.1f), texDarkWood);
    addBox(statics, vec3(2.1f, 1.1f, -5.8f), vec3(3.9f, 2.1f, -5.2f), texAquarium);
    addBox(statics, vec3(4.4f, 0.0f, -5.9f), vec3(5.6f, 2.8f, -4.9f), texDarkWood, texArcade);

    // ---------------- cards ----------------
    // Card geometry in its own local space: it lies flat (y = 0), its NEAR edge (the one
    // closest to the player) sits on the local origin and it extends towards -Z.
    // Rotating around the X axis through the origin therefore tilts it up around its near edge.
    vector<Card> cards;
    for (int i = 0; i < 5; i++)
    {
        vector<vec3> v = {vec3(-CARD_W / 2, 0, 0), vec3(CARD_W / 2, 0, 0), vec3(CARD_W / 2, 0, -CARD_H), vec3(-CARD_W / 2, 0, -CARD_H)};
        vector<vec2> t = {vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1)};
        PolygonWithTexture mesh(v, t);
        mesh.setTexture(texCards[i]);
        cards.emplace_back(mesh, vec3((i - 2) * CARD_SPACING, CARD_Y, DECK_Z));
    }

    vector<int> remaining = {0, 1, 2, 3, 4}; // cards still in the deck
    int playedCount = 0;
    mt19937 rng(random_device{}());

    // ---------------- main loop ----------------
    float stand = 0.0f; // current animation parameter 0..1 (shared by camera and cards)
    float lastFrame = (float)glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        float now = (float)glfwGetTime();
        float dt = now - lastFrame;
        lastFrame = now;

        // --- G pressed: send a random card from the deck to the play zone ---
        while (g_pendingDraws > 0)
        {
            g_pendingDraws--;
            if (remaining.empty())
                continue; // 6th press: nothing happens
            uniform_int_distribution<int> pick(0, (int)remaining.size() - 1);
            int idx = pick(rng);
            int cardIndex = remaining[idx];
            remaining.erase(remaining.begin() + idx);

            Card &c = cards[cardIndex];
            c.playPos = vec3((playedCount - 2) * CARD_SPACING, CARD_Y, PLAY_Z);
            c.state = CardState::Moving;
            c.progress = 0.0f;
            playedCount++;
        }

        // --- animation updates ---
        float step = dt / CAMERA_TRANSITION_TIME;
        stand += glm::clamp(g_standTarget - stand, -step, step);
        float s = smooth01(stand);

        for (Card &c : cards)
            if (c.state == CardState::Moving)
            {
                c.progress += dt / CARD_MOVE_TIME;
                if (c.progress >= 1.0f)
                {
                    c.progress = 1.0f;
                    c.state = CardState::Played;
                }
            }

        // --- render ---
        glClearColor(0.05f, 0.04f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int fbw, fbh;
        glfwGetFramebufferSize(window, &fbw, &fbh);
        if (fbh == 0)
            fbh = 1;

        shader.use();
        mat4 projection = perspective(radians(45.0f), (float)fbw / (float)fbh, 0.1f, 60.0f);
        vec3 eye = mix(CAM1_EYE, CAM2_EYE, s);
        vec3 target = mix(CAM1_TARGET, CAM2_TARGET, s);
        vec3 up = normalize(mix(CAM1_UP, CAM2_UP, s));
        mat4 view = lookAt(eye, target, up);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        for (PolygonWithTexture &p : statics)
            p.draw(shader);

        float angle = STAND_ANGLE * s; // 0 = flat on the table, STAND_ANGLE = standing
        for (Card &c : cards)
        {
            mat4 model = translate(mat4(1.0f), c.position());
            model = rotate(model, angle, vec3(1.0f, 0.0f, 0.0f));
            c.mesh.transformation(model);
            c.mesh.draw(shader);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
