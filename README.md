# DDEControl

this class is a wrapper of DDE letting you connect other AP through DDE

please give your AP administrator authority when used

example like this:


CDDEControl dde();

dde.Init();

dde.SetNotifier(pNotify);

dde.Connect(szServer,szTopic);

dde.StartAdfvice(szItem);


// now you can get notification in notifier when value updated

...
