# 25. How to Create a Pull Request (PR)

A **Pull Request (PR)** is a request to add your work from your branch into the `main` branch.

For our project, the workflow is:

```text
Nour   → nour   → Pull Request → main
Alaa   → alaa   → Pull Request → main# Git & GitHub Guide for Collaborators

This guide explains how to work on the **devena** project using Git and GitHub.

It is written for team members who have **never used Git or GitHub before**.

---

# 1. Git vs GitHub

## What is Git?

**Git** is a tool installed on your computer that keeps track of changes made to your project.

For example:

* You modify a file.
* Git detects the modification.
* You create a **commit** to save a version of your work.
* You **push** your commits to GitHub.

## What is GitHub?

**GitHub** is an online platform where our Git repository is stored.

Our repository is:

**`jiheneweslati-up/devena`**

Think of it this way:

```text
Your computer
     ↓
    Git
     ↓
GitHub (online repository)
```

---

# 2. Our Team Structure

We have one main branch and one branch for each collaborator.

```text
                    ┌── nour
                    │
                    ├── alaa
main ───────────────┼── rayan
                    │
                    └── jihene
```

### Our branches

| Collaborator | Branch   |
| ------------ | -------- |
| Nour         | `nour`   |
| Alaa         | `alaa`   |
| Rayan        | `rayan`  |
| Jihene       | `jihene` |

### Important rule

**Do NOT work directly on `main`.**

Each person must work on their own branch:

* **Nour → `nour`**
* **Alaa → `alaa`**
* **Rayan → `rayan`**
* **Jihene → `jihene`**

After finishing a task, the changes are pushed to the person's branch and then a **Pull Request** is created to merge the changes into `main`.

---

# 3. The Basic Git Vocabulary

Before starting, you should understand these five words.

## Repository

The project managed by Git.

Our repository is:

```text
devena
```

## Branch

A separate version of the project where you can work without directly modifying `main`.

Example:

```text
main
nour
alaa
rayan
jihene
```

## Commit

A saved checkpoint of your work.

For example:

```bash
git commit -m "Add login page"
```

A commit is basically saying:

> "Save the changes I have made."

## Push

Upload your commits from your computer to GitHub.

```bash
git push
```

Think:

```text
Computer → GitHub
```

## Pull

Download the latest changes from GitHub and integrate them into your local project.

```bash
git pull
```

Think:

```text
GitHub → Computer
```

---

# 4. Install Git

First, install Git on your computer.

After installation, open:

* **Git Bash** on Windows
* **Terminal** on macOS/Linux

Check that Git is installed:

```bash
git --version
```

You should see something similar to:

```text
git version 2.x.x
```

---

# 5. Configure Your Git Name and Email

Each collaborator should configure their name and email.

This information is used to identify who created each commit.

## Nour

```bash
git config --global user.name "Nour"
git config --global user.email "YOUR_GITHUB_EMAIL"
```

## Alaa

```bash
git config --global user.name "Alaa"
git config --global user.email "YOUR_GITHUB_EMAIL"
```

## Rayan

```bash
git config --global user.name "Rayan"
git config --global user.email "YOUR_GITHUB_EMAIL"
```

## Jihene

```bash
git config --global user.name "Jihene"
git config --global user.email "YOUR_GITHUB_EMAIL"
```

Replace:

```text
YOUR_GITHUB_EMAIL
```

with the email associated with your GitHub account.

You can verify the configuration with:

```bash
git config --global user.name
git config --global user.email
```

### Important

Your Git name and email are **not your GitHub password**.

They only identify you as the author of your commits.

---

# 6. Access to the GitHub Repository

Before working on the project, each collaborator must be added to the GitHub repository as a collaborator.

The repository owner should give access to:

* Nour
* Alaa
* Rayan
* Jihene

Each person must accept the GitHub invitation.

Without repository access, you will not be able to push your work.

---

# 7. Authentication: HTTPS or SSH

There are two common ways to connect Git to GitHub:

## Option 1 — HTTPS

```text
https://github.com/jiheneweslati-up/devena.git
```

## Option 2 — SSH

```text
git@github.com:jiheneweslati-up/devena.git
```

Both methods are valid.

**SSH is recommended for regular development** because once it is configured, you do not normally need to enter credentials for every Git operation.

---

# 8. Option A — HTTPS Authentication

You can clone the repository using HTTPS:

```bash
git clone https://github.com/jiheneweslati-up/devena.git
```

Then:

```bash
cd devena
```

When GitHub asks you to authenticate, follow the authentication instructions shown by Git.

### Important

GitHub does **not** accept your normal GitHub account password for Git operations over HTTPS.

If Git asks for a password, you may need to use a **Personal Access Token (PAT)** instead.

A Personal Access Token acts as a password for Git operations.

### Security rule

**Never send your Personal Access Token to another person.**

Do not put it:

* in the README
* in the source code
* in screenshots
* in Git commands that you share publicly
* in a `.env` file that is committed to Git

---

# 9. Option B — SSH Authentication

SSH is the recommended option for developers who will use Git regularly.

## Step 1 — Check SSH

Run:

```bash
ssh -V
```

If SSH is installed, you should see something similar to:

```text
OpenSSH_...
```

---

## Step 2 — Generate an SSH key

Run:

```bash
ssh-keygen -t ed25519 -C "YOUR_GITHUB_EMAIL"
```

Press **Enter** when asked where to save the key.

You can also create a passphrase for additional security.

---

## Step 3 — Start the SSH agent

On Git Bash:

```bash
eval "$(ssh-agent -s)"
```

---

## Step 4 — Add your SSH key

```bash
ssh-add ~/.ssh/id_ed25519
```

---

## Step 5 — Display your public key

Run:

```bash
cat ~/.ssh/id_ed25519.pub
```

You will see something similar to:

```text
ssh-ed25519 AAAA........ YOUR_GITHUB_EMAIL
```

Copy the **entire line**.

---

## Step 6 — Add the key to GitHub

Go to your GitHub account.

Open:

**Settings → SSH and GPG keys → New SSH key**

Give the key a name such as:

```text
My Laptop
```

Paste your public key and save it.

### Important security rule

You can share the **public key**.

Never share:

```text
id_ed25519
```

The private key must remain private.

---

## Step 7 — Test SSH

Run:

```bash
ssh -T git@github.com
```

If everything is correctly configured, GitHub should recognize your account.

---

# 10. Clone the Project

You only need to clone the repository **once**.

## Using HTTPS

```bash
git clone https://github.com/jiheneweslati-up/devena.git
```

## Using SSH

```bash
git clone git@github.com:jiheneweslati-up/devena.git
```

Then enter the project folder:

```bash
cd devena
```

---

# 11. Check Your Branch

Before doing any work, always check which branch you are currently using:

```bash
git branch
```

You may see:

```text
* main
  nour
  alaa
  rayan
  jihene
```

The `*` indicates your current branch.

### Very important

Before modifying files, make sure the `*` is next to **your own branch**.

---

# 12. Nour's Setup

Nour must work on:

```text
nour
```

After cloning the repository:

```bash
cd devena
git fetch
git checkout nour
```

If the branch only exists on GitHub and not locally, use:

```bash
git checkout -b nour origin/nour
```

Then verify:

```bash
git branch
```

You should see:

```text
* nour
```

Nour should now work only on the `nour` branch.

---

# 13. Alaa's Setup

Alaa must work on:

```text
alaa
```

After cloning:

```bash
cd devena
git fetch
git checkout alaa
```

If necessary:

```bash
git checkout -b alaa origin/alaa
```

Verify:

```bash
git branch
```

The result should show:

```text
* alaa
```

Alaa should now work only on the `alaa` branch.

---

# 14. Rayan's Setup

Rayan must work on:

```text
rayan
```

After cloning:

```bash
cd devena
git fetch
git checkout rayan
```

If necessary:

```bash
git checkout -b rayan origin/rayan
```

Verify:

```bash
git branch
```

The result should show:

```text
* rayan
```

Rayan should now work only on the `rayan` branch.

---

# 15. Jihene's Setup

Jihene must work on:

```text
jihene
```

After cloning:

```bash
cd devena
git fetch
git checkout jihene
```

If necessary:

```bash
git checkout -b jihene origin/jihene
```

Verify:

```bash
git branch
```

The result should show:

```text
* jihene
```

Jihene should now work only on the `jihene` branch.

---

# 16. The Normal Workflow

Once your branch is correctly configured, the workflow is very simple.

```text
1. Update your branch
        ↓
2. Work on your files
        ↓
3. Check your changes
        ↓
4. Add your changes
        ↓
5. Create a commit
        ↓
6. Push your branch
        ↓
7. Create a Pull Request
        ↓
8. Review
        ↓
9. Merge into main
```

---

# 17. Step 1 — Make Sure You Are on the Correct Branch

Before starting work:

```bash
git branch
```

For example, Nour should see:

```text
* nour
  main
```

Alaa should see:

```text
* alaa
  main
```

Rayan:

```text
* rayan
  main
```

Jihene:

```text
* jihene
  main
```

**Never skip this step.**

---

# 18. Step 2 — Get the Latest Changes

Before starting new work, update your local repository:

```bash
git pull
```

This downloads the latest changes from your current branch.

---

# 19. Step 3 — Work on Your Task

Now modify the project files.

For example, you might:

* create a new file
* modify an existing file
* fix a bug
* add a feature
* improve the interface
* update documentation

Work normally with your preferred editor or IDE.

---

# 20. Step 4 — Check Your Changes

When you finish a part of your work, run:

```bash
git status
```

Git will show which files have been modified.

Example:

```text
modified: src/login.js
modified: src/style.css
```

---

# 21. Step 5 — Add Your Changes

To add all modified files:

```bash
git add .
```

You can check again:

```bash
git status
```

The files should now appear as staged changes.

---

# 22. Step 6 — Create a Commit

Create a commit describing what you did.

Example:

```bash
git commit -m "Add login page"
```

Other examples:

```bash
git commit -m "Fix authentication bug"
```

```bash
git commit -m "Add dashboard interface"
```

```bash
git commit -m "Update project documentation"
```

A commit is a saved checkpoint of your work.

---

# 23. Step 7 — Push Your Branch

After committing:

```bash
git push
```

Your changes are now uploaded to your branch on GitHub.

For example:

```text
Nour's computer
      ↓
    commit
      ↓
    git push
      ↓
GitHub / nour
```

---

# 24. Exact Push Commands for Each Person

## Nour

```bash
git checkout nour
git add .
git commit -m "Describe Nour's changes"
git push
```

## Alaa

```bash
git checkout alaa
git add .
git commit -m "Describe Alaa's changes"
git push
```

## Rayan

```bash
git checkout rayan
git add .
git commit -m "Describe Rayan's changes"
git push
```

## Jihene

```bash
git checkout jihene
git add .
git commit -m "Describe Jihene's changes"
git push
```

---

# 25. Pull Requests

After pushing your work, **do not directly merge it into `main`**.

Instead, create a **Pull Request (PR)**.

A Pull Request means:

> "I finished my work. Please review my changes and consider adding them to `main`."

For example:

```text
nour ───────────────→ Pull Request → main

alaa ───────────────→ Pull Request → main

rayan ──────────────→ Pull Request → main

jihene ─────────────→ Pull Request → main
```

---

# 26. How to Create a Pull Request

After pushing your branch, follow these steps on GitHub.

## Step 1 — Open the Repository

Go to:

```text
https://github.com/jiheneweslati-up/devena
```

Make sure you are logged into the GitHub account that has access to the repository.

---

## Step 2 — Open Pull Requests

At the top of the repository, click:

**Pull requests**

Then click:

**New pull request**

Sometimes GitHub will automatically show a button such as:

**Compare & pull request**

If you see it after pushing your branch, you can click it directly.

---

## Step 3 — Select the Correct Branches

You will see two branches:

```text
base
compare
```

The **base branch must always be `main`**.

The **compare branch must be your own branch**.

### Nour

```text
base: main
compare: nour
```

### Alaa

```text
base: main
compare: alaa
```

### Rayan

```text
base: main
compare: rayan
```

### Jihene

```text
base: main
compare: jihene
```

The direction should always be:

```text
your branch → main
```

For example:

```text
nour → main
```

**Do not accidentally select `main` as the compare branch.**

---

## Step 4 — Check the Changes

GitHub will show you the files and changes that will be included in the Pull Request.

Review them carefully.

Make sure that:

* the changes belong to your task
* there are no accidental files
* you are comparing your branch with `main`
* your work is complete

If everything is correct, continue.

---

## Step 5 — Write a Pull Request Title

Give your Pull Request a clear title.

Good examples:

```text
Add login functionality
```

```text
Implement project management
```

```text
Fix dashboard bug
```

```text
Update README documentation
```

Avoid titles such as:

```text
changes
```

or:

```text
test
```

because they do not explain what was changed.

---

## Step 6 — Write a Description

Briefly explain what you changed.

For example:

```text
## Changes

- Added the login page
- Added form validation
- Updated the CSS

## Testing

- Tested the login form
- Tested validation messages
```

You don't need to write a very long description.

Just explain the important changes.

---

## Step 7 — Create the Pull Request

When everything is ready, click:

**Create pull request**

Your Pull Request is now created.

For example:

```text
Nour
nour → main
```

means:

> Nour is asking the team to review her changes and merge them into `main`.

---

# 27. What Happens After Creating a Pull Request?

After creating the Pull Request, the team can review your work.

The Pull Request allows the team to:

* see which files were changed
* see exactly what was added or removed
* leave comments
* request changes
* approve the Pull Request
* merge the changes into `main`

The person who created the Pull Request should wait for the review before the changes are merged.

---

# 28. If Changes Are Requested

Sometimes a team member may ask you to modify something before your Pull Request is approved.

**Do not create a new Pull Request.**

Stay on the same branch and make the requested changes.

For example, if Nour has a Pull Request:

```text
nour → main
```

Nour continues working on:

```bash
git checkout nour
```

Then:

```bash
git pull
```

Make the requested changes.

Then:

```bash
git status
git add .
git commit -m "Fix requested changes"
git push
```

The existing Pull Request will automatically be updated with the new commit.

---

# 29. Reviewing a Pull Request

Before merging a Pull Request, the team should review the changes.

For example, when Nour creates:

```text
nour → main
```

the team can:

1. Open the Pull Request.
2. Review the changed files.
3. Check the code.
4. Leave comments if necessary.
5. Ask Nour to make changes if needed.
6. Approve the Pull Request when everything is correct.

---

# 30. Merging a Pull Request

When the Pull Request has been reviewed and approved, an authorized team member can click:

**Merge pull request**

Then:

**Confirm merge**

The changes from the collaborator's branch will be added to `main`.

For example:

```text
nour
  │
  │ Pull Request
  ↓
main
```

The same applies to:

```text
alaa → main
rayan → main
jihene → main
```

---

# 31. Important: Do Not Merge Your Own Work Without Review

The normal team workflow is:

```text
1. Work on your branch
        ↓
2. Commit
        ↓
3. Push
        ↓
4. Create Pull Request
        ↓
5. Team reviews your work
        ↓
6. Pull Request is approved
        ↓
7. Merge into main
```

This helps the team avoid accidentally adding incorrect or unfinished code to `main`.

---

# 32. Working on the Same Branch Later

If you already have a branch and want to continue working on it:

## Nour

```bash
git checkout nour
git pull
```

## Alaa

```bash
git checkout alaa
git pull
```

## Rayan

```bash
git checkout rayan
git pull
```

## Jihene

```bash
git checkout jihene
git pull
```

Then continue working.

---

# 33. Important Team Rule — Do Not Push to Main

Do **NOT** use:

```bash
git push origin main
```

unless you are specifically responsible for managing `main`.

The normal team workflow is:

```text
Your branch
    ↓
commit
    ↓
push
    ↓
Pull Request
    ↓
review
    ↓
main
```

---

# 34. Useful Git Commands

## Check your current status

```bash
git status
```

## See your branches

```bash
git branch
```

## Download information about remote branches

```bash
git fetch
```

## Change branch

```bash
git checkout branch-name
```

Example:

```bash
git checkout nour
```

## Download and integrate changes

```bash
git pull
```

## Add all changes

```bash
git add .
```

## Create a commit

```bash
git commit -m "Your message"
```

## Upload your commits

```bash
git push
```

## See previous commits

```bash
git log --oneline
```

---

# 35. The Most Important Commands to Remember

For everyday work, you mainly need:

```bash
git status
git pull
git add .
git commit -m "Describe your changes"
git push
```

The normal sequence is:

```bash
git pull
```

Work on your files.

Then:

```bash
git status
git add .
git commit -m "Describe what you changed"
git push
```

After pushing, create a Pull Request:

```text
your branch → main
```

---

# 36. Example: Nour Completes a Task

Suppose Nour has implemented the login page.

First:

```bash
git checkout nour
```

Update the branch:

```bash
git pull
```

After finishing the login page:

```bash
git status
```

Then:

```bash
git add .
```

Create the commit:

```bash
git commit -m "Add login page"
```

Push:

```bash
git push
```

Then Nour creates a Pull Request:

```text
nour → main
```

After the team reviews the Pull Request, it can be merged into `main`.

---

# 37. Example: Alaa Completes a Task

Alaa works on the `alaa` branch:

```bash
git checkout alaa
git pull
```

After completing the task:

```bash
git status
git add .
git commit -m "Implement project form"
git push
```

Then create:

```text
alaa → main
```

through a Pull Request.

---

# 38. Example: Rayan Completes a Task

Rayan works on:

```text
rayan
```

Commands:

```bash
git checkout rayan
git pull
git status
git add .
git commit -m "Add project validation"
git push
```

Then create:

```text
rayan → main
```

through a Pull Request.

---

# 39. Example: Jihene Completes a Task

Jihene works on:

```text
jihene
```

Commands:

```bash
git checkout jihene
git pull
git status
git add .
git commit -m "Update project interface"
git push
```

Then create:

```text
jihene → main
```

through a Pull Request.

---

# 40. What Happens When Changes Are Merged?

Suppose Nour's Pull Request is accepted.

The structure becomes:

```text
main
  ↑
  │
nour
```

Nour's changes are now included in `main`.

The same process applies to:

```text
alaa → main
rayan → main
jihene → main
```

---

# 41. Keeping Your Branch Updated

After other people's Pull Requests are merged into `main`, your branch may become older than `main`.

For example:

```text
main  → contains new changes
nour  → older version
```

Before starting new work, the team should make sure their branch is updated according to the team's chosen integration workflow.

For beginners, **do not experiment with commands such as `git reset --hard`, `git rebase`, or force-push unless the team leader specifically instructs you to do so.**

These commands can cause loss or rewriting of work if used incorrectly.

---

# 42. Merge Conflicts

Sometimes two people modify the same part of the same file.

Git may report a **merge conflict**.

A conflict can look like:

```text
<<<<<<< HEAD
Your version
=======
Other version
>>>>>>> other-branch
```

Do not panic.

The conflicting code must be reviewed and the correct version selected or combined.

After resolving the conflict:

```bash
git add .
git commit -m "Resolve merge conflict"
```

If you are unsure how to resolve a conflict, **ask the team leader before using commands that could delete or overwrite work.**

---

# 43. Common Problems

## Problem 1 — Authentication failed

You may see:

```text
Authentication failed
```

Possible solutions:

* Check that you have access to the repository.
* If using HTTPS, authenticate correctly or use a Personal Access Token.
* If using SSH, check that your SSH key has been added to GitHub.
* Test SSH with:

```bash
ssh -T git@github.com
```

---

## Problem 2 — Permission denied (publickey)

You may see:

```text
Permission denied (publickey)
```

If using SSH:

1. Check that your SSH key exists.
2. Make sure the public key was added to GitHub.
3. Start the SSH agent.
4. Add the key:

```bash
ssh-add ~/.ssh/id_ed25519
```

5. Test:

```bash
ssh -T git@github.com
```

---

## Problem 3 — I don't know which branch I'm on

Run:

```bash
git branch
```

The branch with `*` is your current branch.

For example:

```text
  main
* nour
  alaa
  rayan
  jihene
```

This means you are currently working on `nour`.

---

## Problem 4 — I accidentally worked on main

**Stop before pushing.**

Do not run:

```bash
git push origin main
```

Contact the team leader and explain what happened so the changes can be moved to the correct branch safely.

---

## Problem 5 — Push was rejected

You may see:

```text
rejected
```

Do not immediately use force push.

First stop and ask the team leader what to do, especially if you are not familiar with Git.

---

# 44. Git Security Rules

Never share:

* GitHub passwords
* Personal Access Tokens
* SSH private keys
* API keys
* database passwords
* `.env` files containing secrets

Never commit secrets to GitHub.

For example, do not write:

```text
PASSWORD=123456
```

inside a file that will be committed to the repository.

---

# 45. Before Every Commit

Before committing, check:

```bash
git status
```

Then verify:

```bash
git branch
```

Make sure you are on your own branch:

```text
Nour   → nour
Alaa   → alaa
Rayan  → rayan
Jihene → jihene
```

Then:

```bash
git add .
git commit -m "Describe your changes"
git push
```

After pushing, remember to create a Pull Request:

```text
your branch → main
```

---

# 46. Quick Start — Nour

```bash
git clone git@github.com:jiheneweslati-up/devena.git
cd devena
git checkout -b nour origin/nour
git pull
```

Work on the project.

Then:

```bash
git status
git add .
git commit -m "Describe Nour's changes"
git push
```

Create a Pull Request:

```text
nour → main
```

---

# 47. Quick Start — Alaa

```bash
git clone git@github.com:jiheneweslati-up/devena.git
cd devena
git checkout -b alaa origin/alaa
git pull
```

Work on the project.

Then:

```bash
git status
git add .
git commit -m "Describe Alaa's changes"
git push
```

Create a Pull Request:

```text
alaa → main
```

---

# 48. Quick Start — Rayan

```bash
git clone git@github.com:jiheneweslati-up/devena.git
cd devena
git checkout -b rayan origin/rayan
git pull
```

Work on the project.

Then:

```bash
git status
git add .
git commit -m "Describe Rayan's changes"
git push
```

Create a Pull Request:

```text
rayan → main
```

---

# 49. Quick Start — Jihene

```bash
git clone git@github.com:jiheneweslati-up/devena.git
cd devena
git checkout -b jihene origin/jihene
git pull
```

Work on the project.

Then:

```bash
git status
git add .
git commit -m "Describe Jihene's changes"
git push
```

Create a Pull Request:

```text
jihene → main
```

---

# 50. Our Team Workflow

The complete workflow is:

```text
                    GitHub Repository
                           │
                          main
                           │
          ┌────────────────┼────────────────┐
          │                │                │
        nour              alaa             rayan
          │                │                │
          └────────────────┼────────────────┘
                           │
                         jihene
```

More simply:

```text
Nour   → nour   → Pull Request → main
Alaa   → alaa   → Pull Request → main
Rayan  → rayan  → Pull Request → main
Jihene → jihene → Pull Request → main
```

Each collaborator:

```text
1. Works on their own branch
2. Saves changes with commits
3. Pushes their branch to GitHub
4. Creates a Pull Request
5. Waits for review
6. Changes are merged into main
```

---

# 51. Final Rules for the Team

## Rule 1

**Never work directly on `main`.**

## Rule 2

Each person works on their assigned branch:

```text
Nour   → nour
Alaa   → alaa
Rayan  → rayan
Jihene → jihene
```

## Rule 3

Always check your branch before working:

```bash
git branch
```

## Rule 4

Save your work with commits:

```bash
git add .
git commit -m "Describe your changes"
```

## Rule 5

Push your work to your own branch:

```bash
git push
```

## Rule 6

Use Pull Requests to merge your work into `main`.

## Rule 7

Never share passwords, tokens, or private SSH keys.

## Rule 8

If you are unsure about a Git error, **stop before using destructive commands and ask the team leader.**

---

# 52. The 5 Commands You Should Memorize

For most everyday work, remember:

```bash
git pull
```

```bash
git status
```

```bash
git add .
```

```bash
git commit -m "Describe your changes"
```

```bash
git push
```

That's the basic Git workflow for the **devena** project.

Rayan  → rayan  → Pull Request → main
Jihene → jihene → Pull Request → main
```

You should create a Pull Request **after you have committed and pushed your work**.

---

## Step 1 — Finish Your Work

Make sure your changes are complete.

Check your branch:

```bash
git branch
```

You should be on your assigned branch:

```text
Nour   → nour
Alaa   → alaa
Rayan  → rayan
Jihene → jihene
```

---

## Step 2 — Commit Your Changes

Check your changes:

```bash
git status
```

Add the changes:

```bash
git add .
```

Create a commit:

```bash
git commit -m "Describe what you changed"
```

For example:

```bash
git commit -m "Add login page"
```

---

## Step 3 — Push Your Branch

Push your branch to GitHub:

```bash
git push
```

For example, Nour's changes will be pushed to:

```text
nour
```

Alaa's changes will be pushed to:

```text
alaa
```

Rayan's changes will be pushed to:

```text
rayan
```

Jihene's changes will be pushed to:

```text
jihene
```

---

## Step 4 — Open the GitHub Repository

Go to the project repository:

```text
https://github.com/jiheneweslati-up/devena
```

After pushing your branch, GitHub may display a message such as:

```text
Compare & pull request
```

Click **Compare & pull request**.

If you don't see this button:

1. Open the **Pull requests** tab.
2. Click **New pull request**.

---

## Step 5 — Select the Correct Branches

This is very important.

The **base branch** must be:

```text
main
```

The **compare branch** must be your own branch.

### Nour

```text
base: main
compare: nour
```

### Alaa

```text
base: main
compare: alaa
```

### Rayan

```text
base: main
compare: rayan
```

### Jihene

```text
base: main
compare: jihene
```

The Pull Request should therefore look like:

```text
your branch  ─────────→  main
```

---

## Step 6 — Write a Clear Title

Give your Pull Request a title that explains what you did.

Good examples:

```text
Add login page
```

```text
Implement project management
```

```text
Fix dashboard bug
```

```text
Update project documentation
```

Avoid titles such as:

```text
changes
```

or:

```text
test
```

because they do not clearly explain the work.

---

## Step 7 — Add a Description

In the description, briefly explain what you changed.

For example:

```text
## Changes

- Added the login page
- Added form validation
- Updated the CSS

## Testing

- Tested login form
- Tested validation messages
```

You don't need to write a long description. Just explain the important changes.

---

## Step 8 — Create the Pull Request

After checking everything:

Click:

**Create pull request**

Your Pull Request is now created.

For example:

```text
Nour
nour → main
```

means:

> Nour is asking the team to review her changes and merge them into `main`.

---

# 26. After Creating the Pull Request

Do **not** delete your branch immediately.

The team can review your Pull Request.

Someone may ask you to make changes.

If changes are requested, simply continue working on the **same branch**.

For example, Nour stays on:

```bash
git checkout nour
```

Then make the requested changes:

```bash
git add .
git commit -m "Fix requested changes"
git push
```

The existing Pull Request will automatically be updated.

**You do not need to create another Pull Request.**

---

# 27. Reviewing a Pull Request

Before merging a Pull Request, the team should review the changes.

For example, when Nour creates:

```text
nour → main
```

the team can:

1. Open the Pull Request.
2. Review the changed files.
3. Check the code.
4. Leave comments if necessary.
5. Ask Nour to make changes if needed.
6. Approve the Pull Request when everything is correct.

---

# 28. Merging a Pull Request

When the Pull Request has been reviewed and approved, an authorized team member can click:

**Merge pull request**

Then:

**Confirm merge**

The changes from the collaborator's branch will be added to `main`.

For example:

```text
nour
  │
  │ Pull Request
  ↓
main
```

---

# 29. Important: Do Not Merge Your Own Work Without Review

The normal team workflow is:

```text
1. Work on your branch
        ↓
2. Commit
        ↓
3. Push
        ↓
4. Create Pull Request
        ↓
5. Team reviews your work
        ↓
6. Pull Request is approved
        ↓
7. Merge into main
```

This helps the team avoid accidentally adding incorrect or unfinished code to `main`.

---

# 30. Example — Complete Workflow for Nour

Nour finishes a feature.

First:

```bash
git checkout nour
```

Update the branch:

```bash
git pull
```

Check the changes:

```bash
git status
```

Add the files:

```bash
git add .
```

Create a commit:

```bash
git commit -m "Add login functionality"
```

Push:

```bash
git push
```

Then on GitHub:

```text
base: main
compare: nour
```

Create the Pull Request.

The final workflow is:

```text
Nour's computer
      ↓
    commit
      ↓
    push
      ↓
GitHub: nour
      ↓
Pull Request
      ↓
   Review
      ↓
    main
```

---

# 31. Example — Complete Workflow for Alaa

```bash
git checkout alaa
git pull
git status
git add .
git commit -m "Implement project form"
git push
```

Then create:

```text
base: main
compare: alaa
```

---

# 32. Example — Complete Workflow for Rayan

```bash
git checkout rayan
git pull
git status
git add .
git commit -m "Add project validation"
git push
```

Then create:

```text
base: main
compare: rayan
```

---

# 33. Example — Complete Workflow for Jihene

```bash
git checkout jihene
git pull
git status
git add .
git commit -m "Update project interface"
git push
```

Then create:

```text
base: main
compare: jihene
```

---

# 34. Important Rule About Pull Requests

A Pull Request is **not** the same thing as a commit.

A **commit** saves your work.

```text
git commit
```

A **push** uploads your commits to GitHub.

```text
git push
```

A **Pull Request** asks the team to review and merge your branch into another branch.

```text
nour → main
```

So the complete process is:

```text
WORK
  ↓
COMMIT
  ↓
PUSH
  ↓
PULL REQUEST
  ↓
REVIEW
  ↓
MERGE
```
