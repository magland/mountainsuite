#include "mask_out_artifacts.h"
#include "diskreadmda.h"
#include "diskwritemda.h"
#include <QTime>
#include <math.h>
#include "mlcommon.h"
#include "mda.h"
#include "mlcommon.h"

bool mask_out_artifacts(const QString& timeseries_path, const QString& timeseries_out_path, double threshold, int interval_size)
{
    if ((!threshold) || (!interval_size)) {
        printf("Problem with input parameters. Either threshold or interval_size is zero.\n");
        return false;
    }

    QTime status_timer;
    status_timer.start();

    DiskReadMda X(timeseries_path);
    int M = X.N1();
    int N = X.N2();

    //compute norms of chunks
    Mda norms(M, N / interval_size);
    for (int i = 0; i < N / interval_size; i++) {
        int timepoint = i * interval_size;
        if (status_timer.elapsed() > 5000) {
            printf("mask_out_artifacts compute_norms: %d/%d (%d%%)\n", timepoint, N, (int)(timepoint * 100.0 / N));
            status_timer.restart();
        }
        Mda chunk;
        X.readChunk(chunk, 0, timepoint, M, interval_size);
        for (int m = 0; m < M; m++) {
            double sumsqr = 0;
            for (int aa = 0; aa < interval_size; aa++) {
                sumsqr += chunk.value(m, aa) * chunk.value(m, aa);
            }
            norms.set(sqrt(sumsqr), m, i);
        }
    }

    //determine which chunks to use
    QVector<int> use_it(N / interval_size + 1);
    for (int i = 0; i < use_it.count(); i++)
        use_it[i] = 1;
    for (int m = 0; m < M; m++) {
        QVector<double> vals;
        for (int i = 0; i < norms.N2(); i++) {
            vals << norms.get(m, i);
        }
        double sigma0 = MLCompute::stdev(vals);
        double mean0 = MLCompute::mean(vals);
        printf("For channel %d: mean=%g, stdev=%g, interval size = %d\n", m, mean0, sigma0, interval_size);
        for (int i = 0; i < norms.N2(); i++) {
            if (norms.value(m, i) > mean0 + sigma0 * threshold) {
                use_it[i - 1] = 0; //don't use the neighbor chunks either
                use_it[i] = 0; //don't use the neighbor chunks either
                use_it[i + 1] = 0; //don't use the neighbor chunks either
            }
        }
    }

    //write the data
    int num_timepoints_used = 0;
    int num_timepoints_not_used = 0;
    DiskWriteMda Y;
    Y.open(MDAIO_TYPE_FLOAT32, timeseries_out_path, M, N);
    for (int i = 0; i < N / interval_size; i++) {
        int timepoint = i * interval_size;
        if (status_timer.elapsed() > 5000) {
            printf("mask_out_artifacts write data: %d/%d (%d%%)\n", timepoint, N, (int)(timepoint * 100.0 / N));
            status_timer.restart();
        }
        Mda chunk;
        X.readChunk(chunk, 0, timepoint, M, interval_size);
        if (use_it[i]) {
            num_timepoints_used += interval_size;
            Y.writeChunk(chunk, 0, timepoint);
        }
        else {
            num_timepoints_not_used += interval_size;
        }
    }
    Y.close();

    printf("Using %.2f%% of all timepoints\n", num_timepoints_used * 100.0 / (num_timepoints_used + num_timepoints_not_used));

    return true;
}
