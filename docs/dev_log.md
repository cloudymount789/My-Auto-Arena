# 开发日志

## 2026-04-22

- **Qt GUI 拖拽修复**：`UnitGraphicsItem` 在左键 `mousePressEvent` 中未 `accept()` 时，场景不会将该项作为 mouse grabber，导致收不到后续 `mouseMoveEvent`，表现为无法拖拽。已为左键按下/移动/释放补全 `accept()`，并在移动时校验 `buttons()` 仍含左键。
- **`TileGraphicsItem`**：构造函数中 `setAcceptedMouseButtons(Qt::NoButton)`，避免格子在视觉上大于单位 `boundingRect` 的留白区域抢走点击，从而无法把拖拽交给上层的 `UnitGraphicsItem`。
