import 'dart:async';
import 'dart:math';

import 'package:defer_pointer/defer_pointer.dart';
import 'package:flutter/material.dart';
import 'package:flutter/physics.dart';
import 'package:vector_math/vector_math_64.dart' as math;
import 'package:zenith/widgets/custom_stack.dart';

class TaskSwitcher extends StatefulWidget {
  final double spacing;

  const TaskSwitcher({
    Key? key,
    required this.spacing,
  }) : super(key: key);

  @override
  State<TaskSwitcher> createState() => TaskSwitcherState();
}

var rand = Random();

class TaskSwitcherState extends State<TaskSwitcher> with TickerProviderStateMixin {
  var tasks = <Widget>[];
  bool overview = false;

  AnimationController? controller;

  late BoxConstraints constraints;

  Animation<double>? scaleAnimation;
  Animation<double>? translationAnimation;

  @override
  void dispose() {
    controller?.dispose();
    super.dispose();
  }

  double scale = 1.0;
  double translation = 0.0;

  double get _taskToTaskOffset => constraints.maxWidth + widget.spacing;

  int _taskIndex(double translation) {
    // 0 is the center of the first task.
    var offset = translation + _taskToTaskOffset / 2;
    var index = offset ~/ _taskToTaskOffset;
    return index.clamp(0, tasks.length - 1);
  }

  double _offset(int taskIndex) => taskIndex * _taskToTaskOffset;

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      behavior: HitTestBehavior.translucent,
      onPanDown: (_) {
        if (overview) {
          _stopAnimations();
        }
      },
      onPanUpdate: (DragUpdateDetails details) {
        if (overview) {
          setState(() {
            translation -= details.delta.dx / scale;
          });
        }
      },
      onPanEnd: (DragEndDetails details) {
        if (overview) {
          _flingOverview(details.velocity.pixelsPerSecond.dx);
        }
      },
      child: LayoutBuilder(
        builder: (BuildContext context, BoxConstraints constraints) {
          this.constraints = constraints;
          return CustomStack(
            children: [
              Positioned.fill(
                child: Transform(
                  transform: Matrix4.compose(
                    math.Vector3(-translation * scale, 0, 0),
                    math.Quaternion.identity(),
                    math.Vector3(scale, scale, 1),
                  ),
                  alignment: AlignmentDirectional.center,
                  child: DeferredPointerHandler(
                    child: Stack(
                      clipBehavior: Clip.none,
                      children: _buildTaskWidgets(constraints).toList(),
                    ),
                  ),
                ),
              ),
              Positioned(
                bottom: 0,
                left: 0,
                right: 0,
                child: GestureDetector(
                  behavior: HitTestBehavior.translucent,
                  onPanStart: (_) => _stopAnimations(),
                  onPanUpdate: (DragUpdateDetails details) {
                    if (tasks.isNotEmpty) {
                      setState(() {
                        scale += details.delta.dy / constraints.maxHeight * 2;
                        scale = scale.clamp(0.3, 1);
                        translation -= details.delta.dx / scale;
                      });
                    }
                  },
                  onPanEnd: (DragEndDetails details) {
                    overview = false;

                    var xVelocity = details.velocity.pixelsPerSecond.dx;
                    var yVelocity = details.velocity.pixelsPerSecond.dy;

                    var taskOffset = _offset(_taskIndex(translation));

                    if (xVelocity.abs() > 365) {
                      int targetTaskIndex;
                      if (xVelocity < 0 && translation > taskOffset) {
                        // Next task.
                        targetTaskIndex = (_taskIndex(translation) + 1).clamp(0, tasks.length - 1);
                      } else if (xVelocity > 0 && translation < taskOffset) {
                        // Previous task.
                        targetTaskIndex = (_taskIndex(translation) - 1).clamp(0, tasks.length - 1);
                      } else {
                        // Same task.
                        targetTaskIndex = _taskIndex(translation);
                      }
                      _switchToTaskByIndex(targetTaskIndex);
                    } else if (yVelocity < -200) {
                      _showOverview();
                    } else {
                      _switchToTaskByIndex(_taskIndex(translation));
                    }
                  },
                  child: Container(
                    height: 50,
                    color: Colors.transparent,
                  ),
                ),
              )
            ],
          );
        },
      ),
    );
  }

  Iterable<Widget> _buildTaskWidgets(BoxConstraints constraints) sync* {
    double position = 0;

    for (Widget task in tasks) {
      Widget container = ConstrainedBox(
        constraints: constraints,
        child: Center(
          child: FittedBox(
            child: task,
          ),
        ),
      );
      if (overview) {
        container = GestureDetector(
          behavior: HitTestBehavior.opaque,
          onTap: () => _switchToTask(task),
          child: IgnorePointer(
            child: container,
          ),
        );
      }
      yield Positioned(
        left: position,
        child: DeferPointer(child: container),
      );

      position += constraints.maxWidth + widget.spacing;
    }
  }

  void spawnTask(Widget widget) {
    overview = false;

    if (tasks.isNotEmpty) {
      int currentTaskIndex = _taskIndex(translation);
      var task = tasks.removeAt(currentTaskIndex);
      tasks.add(task);
      translation = _offset(tasks.length - 1);
    }

    tasks.add(widget);
    translationAnimation?.removeListener(_updateTranslationFromAnimation);
    var t = _offset(tasks.length - 1);

    _stopAnimations();
    controller = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 400),
    );

    translationAnimation = Tween(begin: tasks.length > 1 ? translation : _offset(-1), end: t)
        .chain(CurveTween(curve: Curves.easeOutCubic))
        .animate(controller!)
      ..addListener(_updateTranslationFromAnimation);

    scaleAnimation = TweenSequence([
      TweenSequenceItem(
        weight: 30,
        tween: Tween(begin: scale, end: 0.8),
      ),
      TweenSequenceItem(
        weight: 70,
        tween: Tween(begin: 0.8, end: 1.0).chain(CurveTween(curve: Curves.easeOut)),
      ),
    ]).animate(controller!)
      ..addListener(_updateScaleFromAnimation);

    controller!.forward();
  }

  Future<void> stopTask(Widget task) async {
    var index = tasks.indexOf(task);
    if (_taskIndex(translation) != index) {
      // Not visible, no need for animation.
      tasks.remove(task);
      return;
    }

    overview = false;

    var completer = Completer<void>();

    _stopAnimations();
    controller = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 400),
    )..addStatusListener((status) {
        if (status == AnimationStatus.completed) {
          completer.complete();
        }
      });

    double end;
    if (tasks.length == 1) {
      end = _offset(-1);
    } else if (index == 0) {
      end = _offset(1);
    } else {
      end = _offset(_taskIndex(translation) - 1);
    }

    translationAnimation = controller!.drive(CurveTween(curve: Curves.easeOutCubic)).drive(
          Tween(
            begin: translation,
            end: end,
          ),
        )..addListener(_updateTranslationFromAnimation);

    scaleAnimation = controller!.drive(TweenSequence([
      TweenSequenceItem(
        tween: Tween(
          begin: scale,
          end: 0.8,
        ).chain(CurveTween(curve: Curves.linear)),
        weight: 30,
      ),
      TweenSequenceItem(
        tween: Tween(
          begin: 0.8,
          end: 1.0,
        ).chain(CurveTween(curve: Curves.easeOut)),
        weight: 70,
      ),
    ]))
      ..addListener(_updateScaleFromAnimation);

    controller!.forward();

    await completer.future;
    tasks.remove(task);
  }

  void _stopAnimations() {
    controller?.dispose();
    controller = null;
  }

  void _updateScaleFromAnimation() {
    setState(() {
      scale = scaleAnimation!.value;
    });
  }

  void _updateTranslationFromAnimation() {
    setState(() {
      translation = translationAnimation!.value;
    });
  }

  void _flingOverview(double velocity) {
    _stopAnimations();
    controller = AnimationController.unbounded(vsync: this)
      ..addListener(() {
        setState(() {
          translation = controller!.value;
        });
      });

    controller!.animateWith(
      BouncingScrollSimulation(
        position: translation,
        velocity: -velocity / scale,
        leadingExtent: 0,
        trailingExtent: (tasks.length - 1) * _taskToTaskOffset,
        spring: SpringDescription.withDampingRatio(
          mass: 0.5,
          stiffness: 100.0,
          ratio: 1.1,
        ),
      ),
    );
  }

  void _switchToTaskByIndex(int index) {
    setState(() => overview = false);

    _stopAnimations();
    controller = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 400),
    );

    translationAnimation = controller!.drive(
      Tween(
        begin: translation,
        end: _offset(index),
      ).chain(CurveTween(curve: Curves.easeOutCubic)),
    )..addListener(_updateTranslationFromAnimation);

    scaleAnimation = controller!.drive(
      Tween(
        begin: scale,
        end: 1.0,
      ).chain(CurveTween(curve: Curves.easeOutCubic)),
    )..addListener(_updateScaleFromAnimation);

    controller!.forward();
  }

  void _switchToTask(Widget task) => _switchToTaskByIndex(tasks.indexOf(task));

  void _showOverview() {
    overview = true;

    _stopAnimations();
    controller = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 400),
    );

    translationAnimation = controller!.drive(CurveTween(curve: Curves.easeOutCubic)).drive(
          Tween(
            begin: translation,
            end: _offset(_taskIndex(translation)),
          ),
        )..addListener(_updateTranslationFromAnimation);

    scaleAnimation = controller!.drive(CurveTween(curve: Curves.easeOutCubic)).drive(
          Tween(
            begin: scale,
            end: 0.6,
          ),
        )..addListener(_updateScaleFromAnimation);

    controller!.forward();
  }
}
