#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <iostream>
#include <string>
#include <vector>

static const int SCR_WIDTH = 480;
static const int SCR_HEIGHT = 272;

struct RawParticle
{
    float decay = 0.f;
    float decayRate = 0.f;
    float x = 0.f;
    float y = 0.f;
    float vx = 0.f;
    float vy = 0.f;
};

class ParticleSource
{
public:
    ParticleSource(float x, float y, float w, float h, int max)
        : m_spawnX(x)
        , m_spawnY(y)
        , m_spawnW(w)
        , m_spawnH(h)
        , m_decayCoefficient(2.5f)
        , m_velocityCoefficient(50.f)
        , m_repeat(true)
        , m_opacity(1.f)
    {
        m_particles.resize(max);
        regenerateParticles();
    }

    void setSpawnArea(float x, float y, float w, float h)
    {
        m_spawnX = x;
        m_spawnY = y;
        m_spawnW = w;
        m_spawnH = h;
    }

    void setDecayCoefficient(float d)
    {
        m_decayCoefficient = d;
        regenerateParticles();
    }

    void setVelocityCoefficient(float v)
    {
        m_velocityCoefficient = v;
        regenerateParticles();
    }

    void setRepeat(bool r) { m_repeat = r; }
    void setOpacity(float o) { m_opacity = o; }

    void update(float dt)
    {
        for (auto& p : m_particles)
        {
            if (p.decay <= 0.f && m_repeat)
            {
                p.decay = 1.f;
                p.x = randomLinear(m_spawnX, m_spawnW);
                p.y = randomLinear(m_spawnY, m_spawnH);

            }
            else
            {
                p.decay -= p.decayRate * dt;
                p.x += p.vx * dt;
                p.y += p.vy * dt;
            }
        }
    }

    void render(SDL_Renderer* renderer, SDL_Texture* dotTexture) const
    {
        SDL_SetTextureBlendMode(dotTexture, SDL_BLENDMODE_ADD);
        for (auto& p : m_particles)
        {
            int alpha = (int)(p.decay * 255.f);
            if (alpha < 0) alpha = 0;
            if (alpha > 255) alpha = 255;
            alpha = (int)(alpha * m_opacity);

            SDL_SetTextureAlphaMod(dotTexture, (Uint8)alpha);
            SDL_Rect dst{(int)p.x - 1, (int)p.y - 1, 2, 2};
            SDL_RenderCopy(renderer, dotTexture, nullptr, &dst);
        }
    }

private:
    static float frand() { return (float)rand() / (float)RAND_MAX; }

    float randomLinear(float p, float length) const
    {
        if (length == 0.f) return p;
        return p + frand() * length;
    }

    void regenerateParticles()
    {
        int n = (int)m_particles.size();
        for (int i = 0; i < n; ++i)
        {
            auto& p = m_particles[i];
            p.decay = (float)i / (float)n;
            p.x = randomLinear(m_spawnX, m_spawnW);
            p.y = randomLinear(m_spawnY, m_spawnH);

            float rnd = frand();
            p.decayRate = rnd / m_decayCoefficient + 0.000001f;

            float vx = frand();
            float vy = frand();
            p.vx = ((2.f * vx) - 1.f) * m_velocityCoefficient;
            p.vy = ((2.f * vy) - 1.f) * m_velocityCoefficient;
        }
    }

    std::vector<RawParticle> m_particles;
    float m_spawnX, m_spawnY, m_spawnW, m_spawnH;
    float m_decayCoefficient;
    float m_velocityCoefficient;
    bool m_repeat;
    float m_opacity;
};

class TailedParticle
{
public:
    TailedParticle(int length)
    : m_particleN(length)
    , m_particleSize(10.f)
    , m_opacity(1.f)
    , m_sparkles(0, 0, 12.f, 12.f, 500)
    {
        m_sparkles.setRepeat(true);
        m_sparkles.setVelocityCoefficient(22.f);
        m_sparkles.setDecayCoefficient(0.7f);
    }
    void moveTo(float x, float y)
    {
        m_positions.push_front({x, y});
        if ((int)m_positions.size() > m_particleN)
        {
            m_positions.pop_back();
        }

        m_sparkles.setSpawnArea(x - m_particleSize / 2.f, y - m_particleSize / 2.f,
                                 m_particleSize, m_particleSize);
    }

    void update(float dt) { m_sparkles.update(dt); }

    void draw(SDL_Renderer* renderer, SDL_Texture* glowTexture)
    {
        SDL_SetTextureBlendMode(glowTexture, SDL_BLENDMODE_ADD);

        int n = m_particleN;
        int count = (int)m_positions.size();
        for (int i = 0; i < count; ++i)
        {

            float progress = (float)(n - i) / (float)n;

            float alpha = 180.f * std::pow(progress, 1.3f) * m_opacity;

            if (alpha < 0.f) alpha = 0.f;
            if (alpha > 255.f) alpha = 255.f;

            SDL_SetTextureAlphaMod(glowTexture, (Uint8)alpha);
            SDL_Rect dst{(int)(m_positions[i].x - m_particleSize / 2.f),
                (int)(m_positions[i].y - m_particleSize / 2.f),
                (int)m_particleSize,
                (int)m_particleSize};
                SDL_RenderCopy(renderer, glowTexture, nullptr, &dst);
        }

        m_sparkles.render(renderer, glowTexture);
    }

    void setOpacity(float o)
    {
        m_opacity = o;
        if (m_opacity > 1.f) m_opacity = 1.f;
        if (m_opacity < 0.f) m_opacity = 0.f;
    }

private:
    struct Pos { float x, y; };

    int m_particleN;
    float m_particleSize;
    float m_opacity;
    std::deque<Pos> m_positions;
    ParticleSource m_sparkles;
};

class InfinityScreen
{
public:
    InfinityScreen() : m_t(0.f), m_particles(470), m_opacity(1.f) {}

    void update(float dt)
    {
        m_particles.setOpacity(m_opacity);
        m_t += dt * 0.5f;

        float t = m_t;
        float scale = 2.0f / (3.0f - cosf(2.0f * t));

        float modelx = scale * cosf(t);
        float modely = scale * sinf(2.f * t) / 2.f;

        float x = 245.f + (modelx * 212.f);
        float y = 136.f + (modely * 215.f);

        m_particles.moveTo(x, y);
        m_particles.update(dt);
    }

    void render(SDL_Renderer* renderer, SDL_Texture* glowTexture)
    {
        m_particles.draw(renderer, glowTexture);
    }

private:
    float m_t;
    TailedParticle m_particles;
    float m_opacity;
};

class BackgroundView
{
public:
    BackgroundView(SDL_Texture* left, SDL_Texture* right)
        : m_left(left), m_right(right)
    {
        SDL_QueryTexture(m_left, nullptr, nullptr, &m_lw, &m_lh);
        SDL_QueryTexture(m_right, nullptr, nullptr, &m_rw, &m_rh);
    }

    void render(SDL_Renderer* renderer)
    {
        float combined = (float)(m_lw + m_rw);
        float leftX = -(combined - SCR_WIDTH) / 2.f;
        float rightX2 = SCR_WIDTH + (combined - SCR_WIDTH) / 2.f;

        SDL_Rect leftDst{(int)leftX, 0, (int)(SCR_WIDTH / 2.f - leftX), SCR_HEIGHT};
        SDL_Rect rightDst{(int)(SCR_WIDTH / 2.f), 0, (int)(rightX2 - SCR_WIDTH / 2.f), SCR_HEIGHT};

        SDL_SetTextureBlendMode(m_left, SDL_BLENDMODE_NONE);
        SDL_SetTextureBlendMode(m_right, SDL_BLENDMODE_NONE);
        SDL_RenderCopy(renderer, m_left, nullptr, &leftDst);
        SDL_RenderCopy(renderer, m_right, nullptr, &rightDst);
    }

private:
    SDL_Texture* m_left;
    SDL_Texture* m_right;
    int m_lw = 0, m_lh = 0, m_rw = 0, m_rh = 0;
};

static SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& path, bool linear)
{
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf)
    {
        std::cerr << "Impossibile caricare " << path << ": " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (tex)
    {
        SDL_SetTextureScaleMode(tex, linear ? SDL_ScaleModeLinear : SDL_ScaleModeNearest);
    }
    return tex;
}

int main(int argc, char** argv)
{

    std::string assetsDir = "/data/homebrew/InfinityIntro/assets";
    if (argc > 1) assetsDir = argv[1];

    srand((unsigned)time(nullptr));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        std::cerr << "SDL_Init fallito: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        std::cerr << "IMG_Init (PNG) fallito: " << IMG_GetError() << std::endl;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0)
    {
        std::cerr << "Mix_OpenAudio fallito: " << Mix_GetError() << std::endl;
    }

    Mix_Music* music = Mix_LoadMUS((assetsDir + "/theme.ogg").c_str());
    if (!music)
    {
        std::cerr << "Impossibile caricare " << assetsDir << "/theme.ogg: " << Mix_GetError()
                  << std::endl;
    }
    else
    {
        Mix_PlayMusic(music, -1);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    SDL_Window* window = SDL_CreateWindow("Infinity",
                                           SDL_WINDOWPOS_CENTERED,
                                           SDL_WINDOWPOS_CENTERED,
                                           1920, 1080,
                                           SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
    if (!window)
    {
        std::cerr << "SDL_CreateWindow fallito: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Renderer* renderer =
        SDL_CreateRenderer(window, -1, 0);
    if (!renderer)
    {
        std::cerr << "SDL_CreateRenderer fallito: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_RenderSetLogicalSize(renderer, SCR_WIDTH, SCR_HEIGHT);

    SDL_Texture* leftTex = loadTexture(renderer, assetsDir + "/parallaxleft.tga", false);
    SDL_Texture* rightTex = loadTexture(renderer, assetsDir + "/parallaxright.tga", false);
    SDL_Texture* glowTex = loadTexture(renderer, assetsDir + "/glow.png", true);

    if (!leftTex || !rightTex)
    {
        std::cerr << "ATTENZIONE: parallaxleft.tga / parallaxright.tga non trovate in '"
                  << assetsDir << "'.\n";
    }
    if (!glowTex)
    {
        std::cerr << "ERRORE: assets/glow.png mancante.\n";
        return 1;
    }

    BackgroundView* background = (leftTex && rightTex) ? new BackgroundView(leftTex, rightTex) : nullptr;
    InfinityScreen infinity;

    bool running = true;
    Uint64 last = SDL_GetPerformanceCounter();

    const float FIXED_DT = 1.0f / 60.0f;
    float accumulator = 0.f;

    while (running)
    {
        SDL_Event ev;
        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_QUIT) running = false;
            if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        Uint64 now = SDL_GetPerformanceCounter();
        float dt = (float)(now - last) / (float)SDL_GetPerformanceFrequency();
        last = now;
        if (dt > 0.25f) dt = 0.25f;

        accumulator += dt;
        while (accumulator >= FIXED_DT)
        {
            infinity.update(FIXED_DT);
            accumulator -= FIXED_DT;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (background) background->render(renderer);
        infinity.render(renderer, glowTex);

        SDL_RenderPresent(renderer);
    }

    if (music) { Mix_HaltMusic(); Mix_FreeMusic(music); }
    Mix_CloseAudio();

    if (glowTex) SDL_DestroyTexture(glowTex);
    if (leftTex) SDL_DestroyTexture(leftTex);
    if (rightTex) SDL_DestroyTexture(rightTex);
    delete background;

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
