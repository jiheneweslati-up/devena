# 25. How to Create a Pull Request (PR)

A **Pull Request (PR)** is a request to add your work from your branch into the `main` branch.

For our project, the workflow is:

```text
Nour   → nour   → Pull Request → main
Alaa   → alaa   → Pull Request → main
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
