#include "include/DebugTerminal.h"
#include "include/AppState.h"
#include "include/ShaderSystem.h"
#include "include/Shader.h"
#include "include/UiSystem.h"

#include <sstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/common.hpp>

#include "include/DebugLog.h"

// wraps by words first and then breaks long words if necessary
static std::vector<std::string> wrapTextWidth(const std::string &text,
											  TextRenderer &textRenderer,
											  float scale, float maxWidth) {
	std::vector<std::string> result;

	if (text.empty()) {
		result.push_back("");
		return result;
	}

	std::istringstream stream(text);
	std::string word;
	std::string currentLine;

	while (stream >> word) {
		std::string candidate;

		if (currentLine.empty())
			candidate = word;
		else
			candidate = currentLine + " " + word;

		if (textRenderer.measureTextWidth(candidate, scale) <= maxWidth)
			currentLine = candidate;

		else {

			if (!currentLine.empty()) {
				result.push_back(currentLine);
				currentLine.clear();
			}

			if (textRenderer.measureTextWidth(word, scale) <= maxWidth)
				currentLine = word;
			else {
				std::string chunk;

				for (char c : word) {
					std::string chunkCandidate = chunk + c;

					if (textRenderer.measureTextWidth(chunkCandidate, scale) >
							maxWidth &&
						!chunk.empty()) {
						result.push_back(chunk);
						chunk = std::string(1, c);
					} else
						chunk = chunkCandidate;
				}

				currentLine = chunk;
			}
		}
	}

	if (!currentLine.empty())
		result.push_back(currentLine);

	return result;
}

void drawDebugTerminal(	UiState& uiState,
						ShaderState& shaders,
						const AppState &appState
) {

	if (!appState.debugTerminalOpen)
		return;

	UiRenderer &uiRenderer = uiState.uiRenderer;
	TextRenderer &textRenderer = uiState.textRenderer;

	Shader& uiRectangleShader = shaders.uiRectangleShader;
	Shader& textShader = shaders.textShader;

	float sw = static_cast<float>(appState.framebufferWidth);
	float sh = static_cast<float>(appState.framebufferHeight);

	float panelMargin = 20.0f;

	float panelX = panelMargin;
	float panelW = sw - panelMargin * 2.0f;

	if (panelW < 240.0f)
		panelW = 240.0f;

	float panelH = sh * 0.38f;

	if (panelH < 140.0f)
		panelH = 140.0f;
	if (panelH > 360.0f)
		panelH = 360.0f;

	float panelY = sh - panelH - panelMargin;

	uiRenderer.drawRectangle(uiRectangleShader, panelX, panelY, panelW, panelH,
							 glm::vec3(0.0f, 0.0f, 0.0f), 0.78f, appState.framebufferWidth,
							 appState.framebufferHeight);

	float textScale = 0.65f;
	float padding = 14.0f;
	float lineHeight = 17.0f;

	textRenderer.renderText(textShader, "DEBUG TERMINAL [F1]", panelX + padding,
							panelY + panelH - 28.0f, textScale,
							glm::vec3(0.75f, 1.0f, 0.75f), appState.framebufferWidth,
							appState.framebufferHeight);

	const std::vector<std::string> &lines = DebugLog::getLines();

	if (lines.empty()) {

		textRenderer.renderText(textShader, "No debug messages yet.",
								panelX + padding, panelY + panelH - 55.0f,
								textScale, glm::vec3(0.85f, 0.9f, 0.85f),
								appState.framebufferWidth, appState.framebufferHeight);

		return;
	}

	float maxTextWidth = panelW - padding * 2.0f;
	float startY = panelY + panelH - 55.0f;
	float bottomY = panelY + padding;

	std::vector<std::string> wrappedLines;

	for (const std::string &line : lines) {
		std::vector<std::string> wrapped =
			wrapTextWidth(line, textRenderer, textScale, maxTextWidth);
		wrappedLines.insert(wrappedLines.end(), wrapped.begin(), wrapped.end());
	}

	int maxVisibleLines = static_cast<int>((startY - bottomY) / lineHeight);
	if (maxVisibleLines <= 0)
		return;

	int startIndex =
		glm::max(0, static_cast<int>(wrappedLines.size()) - maxVisibleLines);

	float y = startY;

	for (int i = startIndex; i < (int)wrappedLines.size(); ++i) {
		textRenderer.renderText(textShader, wrappedLines[i], panelX + padding,
								y, textScale, glm::vec3(0.85f, 0.9f, 0.85f),
								appState.framebufferWidth, appState.framebufferHeight);

		y -= lineHeight;

		if (y < bottomY)
			break;
	}
}
