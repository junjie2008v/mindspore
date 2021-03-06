# Copyright 2019 Huawei Technologies Co., Ltd
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import numpy as np

import mindspore as ms
import mindspore.nn as nn
from mindspore import Tensor
from mindspore import context
from mindspore.common.api import _executor
from mindspore.ops import composite as C
from mindspore.ops import operations as P
from tests.ut.python.ops.test_math_ops import VirtualLoss


class NetWithLoss(nn.Cell):
    def __init__(self, network, strategy3=None):
        super(NetWithLoss, self).__init__()
        self.loss = P.SoftmaxCrossEntropyWithLogits().set_strategy(strategy3)
        self.network = network

    def construct(self, x, y, b):
        predict = self.network(x, y)
        return self.loss(predict, b)[0]


class GradWrap(nn.Cell):
    def __init__(self, network):
        super(GradWrap, self).__init__()
        self.network = network

    def construct(self, x, y, b):
        return C.grad_all(self.network)(x, y, b)


def compile(net, x, y, b):
    net.set_auto_parallel()
    _executor.compile(net, x, y, b)


def test_softmax_cross_entropy_loss():
    class Net(nn.Cell):
        def __init__(self, strategy1, strategy2):
            super().__init__()
            self.matmul = P.MatMul(transpose_b=True).set_strategy(strategy1)
            self.gelu = P.Gelu().set_strategy(strategy2)

        def construct(self, x, y):
            out = self.matmul(x, y)
            out = self.gelu(out)
            return out

    context.set_auto_parallel_context(device_num=8, global_rank=0)
    strategy1 = ((4, 1), (2, 1))
    strategy2 = ((4, 2),)
    strategy3 = ((8, 1), (8, 1))
    net = GradWrap(NetWithLoss(Net(strategy1, strategy2), strategy3))
    context.set_auto_parallel_context(parallel_mode="semi_auto_parallel")

    x = Tensor(np.ones([64, 32]), dtype=ms.float32)
    y = Tensor(np.ones([64, 32]), dtype=ms.float32)
    b = Tensor(np.ones([64, 64]), dtype=ms.float32)
    compile(net, x, y, b)


def test_softmax_cross_entropy_loss_repeated_calculation():
    class Net(nn.Cell):
        def __init__(self, strategy1, strategy2):
            super().__init__()
            self.matmul = P.MatMul(transpose_b=True).set_strategy(strategy1)
            self.gelu = P.Gelu().set_strategy(strategy2)

        def construct(self, x, y):
            out = self.matmul(x, y)
            out = self.gelu(out)
            return out

    context.set_auto_parallel_context(device_num=8, global_rank=0)
    strategy1 = ((4, 1), (2, 1))
    strategy2 = ((4, 2),)
    strategy3 = ((2, 1), (2, 1))
    net = GradWrap(NetWithLoss(Net(strategy1, strategy2), strategy3))
    context.set_auto_parallel_context(parallel_mode="semi_auto_parallel")

    x = Tensor(np.ones([64, 32]), dtype=ms.float32)
    y = Tensor(np.ones([64, 32]), dtype=ms.float32)
    b = Tensor(np.ones([64, 64]), dtype=ms.float32)
    compile(net, x, y, b)


def test_softmax_cross_entropy_loss_auto_batch_parallel():
    class Net(nn.Cell):
        def __init__(self):
            super().__init__()
            self.matmul = P.MatMul(transpose_b=True)
            self.gelu = P.Gelu()

        def construct(self, x, y):
            out = self.matmul(x, y)
            out = self.gelu(out)
            return out

    context.set_auto_parallel_context(device_num=8, global_rank=0)
    net = GradWrap(NetWithLoss(Net()))
    context.set_auto_parallel_context(parallel_mode="semi_auto_parallel")

    x = Tensor(np.ones([64, 32]), dtype=ms.float32)
    y = Tensor(np.ones([64, 32]), dtype=ms.float32)
    b = Tensor(np.ones([64, 64]), dtype=ms.float32)
    compile(net, x, y, b)
