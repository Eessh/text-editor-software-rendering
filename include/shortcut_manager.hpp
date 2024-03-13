#pragma once

#include <string>
#include <unordered_map>
#include <vector>

enum class Command
{
  COPY,
  PASTE,
  CUT,
  MOVE_TO_START_OF_BUFFER,
  MOVE_TO_END_OF_BUFFER,
  MOVE_TO_START_OF_LINE,
  MOVE_TO_END_OF_LINE,
  MOVE_TO_START_OF_WORD,
  MOVE_TO_END_OF_WORD,
  MOVE_TO_START_OF_NEXT_WORD,
  MOVE_TO_END_OF_NEXT_WORD,
  MOVE_TO_START_OF_PREVIOUS_WORD,
  MOVE_TO_END_OF_PREVIOUS_WORD,
  UNDO,
  REDO
};

class KeyBinding
{
public:
  std::string modifiers;
  std::string keys;
  Command command;
};

const std::vector<KeyBinding> DefaultShortcuts = {
  {"CTRL", "C", Command::COPY},
  {"CTRL", "V", Command::PASTE},
  {"CTRL", "X", Command::CUT},
};

class ShortcutManager
{
public:
  ShortcutManager(const ShortcutManager& manager) = delete;
  ShortcutManager(ShortcutManager&& manager) = delete;
  ShortcutManager operator=(const ShortcutManager& manager) = delete;
  ShortcutManager operator=(ShortcutManager&& manager) = delete;

  ~ShortcutManager() noexcept = default;

  static void create_instance() noexcept;

  [[nodiscard]] static ShortcutManager* get_instance() noexcept;

  static void delete_instance() noexcept;

  [[nodiscard]] bool add_keybinding(const std::string& modifiers,
                                    const std::string keys,
                                    const Command& command) noexcept;

  [[nodiscard]] bool remove_keybinding(const std::string& modifiers,
                                       const std::string& keys);

private:
  ShortcutManager() noexcept = default;
  static ShortcutManager* _instance;

  std::unordered_map<Command, std::string> keybindings;
};