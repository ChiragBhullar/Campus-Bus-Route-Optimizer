# Django Portfolio — Setup & Run

## Quick Start

```bash
# 1. Create and activate a virtual environment
python -m venv venv
source venv/bin/activate      # Windows: venv\Scripts\activate

# 2. Install dependencies
pip install -r requirements.txt

# 3. Set up environment variables
cp .env.example .env
# Edit .env if needed (defaults work for local dev)

# 4. Run migrations and seed sample data
python manage.py makemigrations portfolio_app
python manage.py migrate

# 5. Create superuser for the admin panel
python manage.py createsuperuser

# 6. Collect static files
python manage.py collectstatic --noinput

# 7. Start the dev server
python manage.py runserver
```

Open http://127.0.0.1:8000 — you're live!

Admin panel: http://127.0.0.1:8000/admin

---

## Project Structure

```
portfolio/
├── manage.py
├── requirements.txt
├── .env.example
│
├── portfolio_site/          # Django project config
│   ├── settings/
│   │   └── base.py
│   ├── urls.py
│   └── wsgi.py
│
└── portfolio_app/           # Main app
    ├── models.py            # Profile, Project, Skill, Experience, Testimonial, ContactMessage
    ├── views.py             # Home, About, Projects, ProjectDetail, Contact
    ├── urls.py
    ├── admin.py
    ├── forms.py
    ├── migrations/
    │   ├── 0001_initial.py  (auto-generated)
    │   └── 0002_seed_data.py
    ├── templates/portfolio_app/
    │   ├── base.html
    │   ├── home.html
    │   ├── about.html
    │   ├── projects.html
    │   ├── project_detail.html
    │   └── contact.html
    └── static/portfolio_app/
        ├── css/main.css
        └── js/main.js
```

## Adding Content

Everything is managed through the **Django admin panel**:

- **Profile** — your name, bio, photo, social links, availability
- **Projects** — add thumbnails, tech stack, live/GitHub links, mark as featured
- **Skill Categories + Skills** — grouped with proficiency bars
- **Experience** — work history and education for the timeline
- **Testimonials** — quotes from colleagues/clients
- **Contact Messages** — view and manage inbound messages

## Pages

| URL | Description |
|-----|-------------|
| `/` | Hero, featured projects, skills, experience preview, testimonials, CTA |
| `/about/` | Full bio, skills, work + education timeline |
| `/projects/` | All projects grid |
| `/projects/<slug>/` | Project detail with description and related |
| `/contact/` | Contact form + social links |
| `/admin/` | Django admin |

## Design

- **Dark theme** with near-black backgrounds and violet accent
- **DM Sans** (UI) + **DM Serif Display** (headings) + **DM Mono** (code/labels)
- Animated skill bars, scroll fade-ins, spinning avatar ring
- Fully responsive — mobile hamburger menu included
- WhiteNoise for static files in production
