import { test, expect } from "@playwright/test";

test.describe("EdgeGate Dashboard", () => {
  test("overview page loads", async ({ page }) => {
    await page.goto("/");
    await expect(page.getByRole("heading", { name: "Network Overview" })).toBeVisible();
  });

  test("navigation works", async ({ page }) => {
    await page.goto("/");
    await page.getByRole("link", { name: "DNS Explorer" }).click();
    await expect(page.getByRole("heading", { name: "DNS Explorer" })).toBeVisible();
  });

  test("policies page has create form", async ({ page }) => {
    await page.goto("/policies");
    await expect(page.getByRole("heading", { name: "Policy Monitor" })).toBeVisible();
    await expect(page.getByPlaceholder("e.g. ads.example.com")).toBeVisible();
  });
});
